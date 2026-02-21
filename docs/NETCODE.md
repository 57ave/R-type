# Network Lag Compensation

This document describes the client-side prediction, server reconciliation, and entity interpolation systems used to compensate for network latency in R-Type multiplayer.

---

## Overview

The server is authoritative: it runs the game simulation at 60 Hz and broadcasts world snapshots at 30 Hz. Without compensation, players would experience input delay equal to RTT (50-150ms over the internet) and remote entities would teleport between snapshot positions.

Three systems work together to solve this:

| System | Target | Effect |
|--------|--------|--------|
| Client-side prediction | Local player | Movement feels instant |
| Server reconciliation | Local player | Corrects prediction drift |
| Entity interpolation | Remote entities | Smooth movement between snapshots |

---

## Protocol Extensions

### ClientInput

```
uint8_t  playerId
uint8_t  inputMask      (bits: 0=up, 1=down, 2=left, 3=right, 4=fire)
uint8_t  chargeLevel    (0-5)
uint32_t inputSeq       (monotonic counter, added for lag compensation)
```

### SnapshotHeader

```
uint32_t entityCount
uint32_t snapshotSeq       (monotonic counter for ordering)
uint8_t  playerAckCount    (number of PlayerInputAck entries following)
```

### PlayerInputAck

```
uint8_t  playerId
uint32_t lastProcessedInputSeq
```

### Wire Format

```
[PacketHeader] [SnapshotHeader] [PlayerInputAck x N] [EntityState x M]
```

---

## Client-Side Prediction

When the local player presses a movement key, the client applies the movement immediately without waiting for the server:

1. Build `inputMask` from keyboard state
2. Apply `applyMovementInput()` to predicted position (speed=500 px/s, same as server)
3. Store `{seq, inputMask, dt}` in `pendingInputs_` buffer (max 120 entries)
4. Send `CLIENT_INPUT` packet with `inputSeq` to server
5. Write predicted position to ECS `Position` component

The movement function replicates server logic exactly:
- Velocity from bitmask (500 px/s per axis)
- Position += velocity * dt
- Clamp to screen bounds [0, 1820] x [0, 1030]

---

## Server Reconciliation

When a `WORLD_SNAPSHOT` arrives containing the local player's state:

1. Find the `PlayerInputAck` for our player ID
2. Drop all entries from `pendingInputs_` where `seq <= ackedInputSeq`
3. Start from the server's authoritative position
4. Replay all remaining unacknowledged inputs on top of server position
5. Compare reconciled position with current predicted position:
   - If error > 4 pixels: snap to reconciled position
   - Otherwise: keep current prediction (avoids jitter from float precision)

The 4-pixel threshold prevents visual jitter while still correcting meaningful divergence (e.g., hitting a boundary the client didn't account for).

---

## Entity Interpolation

Remote entities (other players, enemies, projectiles) are interpolated between the last two snapshot positions:

1. On each snapshot, shift `current` state to `previous`, store new state as `current`
2. Each frame, compute `t = elapsed_since_last_snapshot / SNAPSHOT_INTERVAL`
3. Lerp: `position = previous + (current - previous) * clamp(t, 0, 1)`
4. No extrapolation: when `t >= 1.0`, hold at current position

This adds ~33ms of visual delay (one snapshot interval) but eliminates teleporting.

---

## RTT Measurement

The client sends `CLIENT_PING` every 1 second with `steady_clock` timestamp in the packet header. The server echoes this timestamp in the `SERVER_PING_REPLY` payload. The client computes:

```
rtt = (now - echoedTimestamp) / 1000.0
smoothedRtt = 0.8 * smoothedRtt + 0.2 * rtt
```

RTT is available via `NetworkManager::getSmoothedRtt()` for UI display or adaptive tuning.

---

## Files Modified

| File | Changes |
|------|---------|
| `server/include/network/RTypeProtocol.hpp` | `inputSeq` on ClientInput, extended SnapshotHeader, PlayerInputAck struct |
| `game/include/network/RTypeProtocol.hpp` | Mirror of server protocol (RType namespace) |
| `server/src/main_improved.cpp` | Track input sequences, serialize acks in snapshots, echo ping timestamp |
| `game/include/managers/NetworkManager.hpp` | Input sequence counter, RTT state, ping timer |
| `game/src/managers/NetworkManager.cpp` | Assign inputSeq, parse acks, reject out-of-order snapshots, RTT computation |
| `game/include/states/NetworkPlayState.hpp` | Prediction/interpolation data structures |
| `game/src/states/NetworkPlayState.cpp` | Prediction, reconciliation, interpolation implementations |

---

## Design Decisions

- **No projectile prediction**: Missiles appear after half-RTT (~25-75ms). Acceptable for a shmup.
- **No extrapolation**: Shmup enemies have erratic movement patterns that don't extrapolate well. Holding at last known position is safer.
- **4-pixel reconciliation threshold**: Prevents visual jitter from float precision differences between client and server.
- **120-input buffer cap**: ~2 seconds at 60fps. Prevents unbounded growth during network stalls.
- **No server-side rewind**: Hit detection uses current positions only. Rewinding would add complexity for minimal benefit in a shmup where projectiles are plentiful.

---

## Verification

```bash
# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

# Basic test (localhost)
./install/bin/r-type_server &
./install/bin/r-type_game

# Latency simulation (Linux)
sudo tc qdisc add dev lo root netem delay 75ms
# Movement should feel instant, remote entities smooth
# RTT should read ~150ms (75ms * 2)

# Remove latency simulation
sudo tc qdisc del dev lo root
```
