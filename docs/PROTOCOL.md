# Network Protocol

All communication between client and server uses **UDP on port 4242**.

TCP was explicitly rejected: its retransmission mechanism introduces variable latency that causes rubber-banding in a real-time game. Packet loss is handled at the protocol level instead.

---

## Packet Header

Every packet starts with a fixed 12-byte header, little-endian, no padding:

```cpp
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t magic;      // Always 0x5254 ('R','T') — drop packet if wrong
    uint8_t  version;    // Protocol version, currently 1
    uint8_t  flags;      // Bit 0 = payload is compressed
    uint16_t type;       // Packet type (see below)
    uint32_t seq;        // Monotonic sequence number per sender
    uint32_t timestamp;  // Sender time in milliseconds
};
#pragma pack(pop)
```

- `magic`: identifies this protocol. Any packet with a wrong magic is silently dropped.
- `version`: incompatible version → drop + log.
- `seq`: used to detect duplicates and out-of-order packets. Old seq numbers are discarded.
- `timestamp`: used by the client to compute round-trip time and interpolation timing.

---

## Packet Types

### Client → Server

| Type ID | Name | Description |
|---|---|---|
| `0x0001` | `CLIENT_HELLO` | Initial connection request |
| `0x0002` | `CLIENT_INPUT` | Player input for current frame |
| `0x0003` | `CLIENT_PING` | Keepalive, sent every 500 ms |
| `0x0004` | `CLIENT_DISCONNECT` | Clean disconnect |

### Server → Client

| Type ID | Name | Description |
|---|---|---|
| `0x0010` | `SERVER_WELCOME` | Connection accepted, assigns player ID |
| `0x0011` | `WORLD_SNAPSHOT` | Full or delta world state, sent at 10 Hz |
| `0x0012` | `ENTITY_SPAWN` | New entity created |
| `0x0013` | `ENTITY_DESTROY` | Entity removed (`uint32_t entityId`) |
| `0x0014` | `PLAYER_DIED` | A player's entity died |
| `0x0015` | `SERVER_PING_REPLY` | Response to CLIENT_PING |
| `0x0016` | `CLIENT_LEFT` | Another client disconnected |

### Room Management

| Type ID | Direction | Name | Description |
|---|---|---|---|
| `0x0020` | C→S | `CREATE_ROOM` | Create a new room |
| `0x0021` | C→S | `JOIN_ROOM` | Join an existing room |
| `0x0022` | C→S | `ROOM_LIST` | Request list of rooms |
| `0x0023` | C→S | `GAME_START` | Host starts the game |
| `0x0024` | C→S | `RENAME_ROOM` | Rename a room |
| `0x0030` | S→C | `ROOM_JOINED` | Confirmation of join |
| `0x0031` | S→C | `ROOM_LIST_REPLY` | List of available rooms |
| `0x0032` | S→C | `ROOM_CREATED` | Room created, returns `uint32_t roomId` |
| `0x0033` | S→C | `ROOM_PLAYERS_UPDATE` | Updated player list for a room |
| `0x0034` | C→S | `CLIENT_TOGGLE_PAUSE` | Player requests pause |
| `0x0035` | S→C | `SERVER_SET_PAUSE` | Server broadcasts pause state |
| `0x0040` | both | `CHAT_MESSAGE` | Chat message (`roomId=0` = global, `>0` = room-specific) |

---

## Key Payloads

### CLIENT_INPUT (3 bytes)

```cpp
struct ClientInput {
    uint8_t playerId;
    uint8_t inputMask;   // bit 0=Up, 1=Down, 2=Left, 3=Right, 4=Fire
    uint8_t chargeLevel; // 0=normal shot, 1-5=charge level
};
```

Sent every frame (up to 60 Hz), but only transmitted when the mask changes.

### WORLD_SNAPSHOT

```cpp
struct SnapshotHeader {
    uint32_t tick;
    uint32_t timestamp;
    uint16_t entityCount;
};
```

Followed by `entityCount` records of:

```cpp
struct EntityState {
    uint32_t id;
    uint8_t  type;           // see Entity Types below
    int16_t  x, y;           // position (quantized)
    int16_t  vx, vy;         // velocity (quantized)
    uint8_t  hp;
    uint8_t  playerLine;
    uint8_t  playerId;
    uint8_t  chargeLevel;
    uint8_t  enemyType;
    uint8_t  projectileType;
};
```

Total: 19 bytes per entity.

---

## Entity Types

```cpp
enum EntityType : uint8_t {
    ENTITY_PLAYER        = 0,
    ENTITY_ENEMY         = 1,
    ENTITY_PLAYER_BULLET = 2,
    ENTITY_ENEMY_BULLET  = 3,
    ENTITY_OBSTACLE      = 4,
    ENTITY_COLLECTABLE   = 5,
};
```

---

## Reliability

UDP does not guarantee delivery or order. The protocol handles this as follows:

- **Duplicate detection**: incoming `seq` numbers are tracked per sender. Duplicates are dropped.
- **Out-of-order packets**: older `seq` values than the last processed are discarded.
- **Keepalive**: client sends `CLIENT_PING` every 500 ms. Server replies with `SERVER_PING_REPLY`.
- **Timeout**: if the server receives nothing from a client for **30 seconds**, it removes the session and broadcasts `CLIENT_LEFT` to the room.

---

## Send Rates

| Packet | Rate |
|---|---|
| `CLIENT_INPUT` | up to 60 Hz (on change only) |
| `WORLD_SNAPSHOT` | 10 Hz |
| `ENTITY_SPAWN` / `ENTITY_DESTROY` | event-driven |
| `CLIENT_PING` | every 500 ms |

---

## Serialization Rules

- All integers: **little-endian**
- No padding between fields (`#pragma pack(push, 1)` / `#pragma pack(pop)`)
- Fixed-size types only (`uint8_t`, `uint16_t`, `int16_t`, `uint32_t`)
- Strings: `uint32_t length` followed by raw bytes (no null terminator)

---

## Error Handling

Both client and server must never crash on a malformed packet. Required behavior:

- Packet too short → drop
- Unknown type → drop + log
- Version mismatch → drop + log
- Unknown entity ID → log + skip
- Payload truncated or inconsistent → drop
