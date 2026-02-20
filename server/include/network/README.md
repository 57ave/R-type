# R-Type Network Protocol

This directory contains the R-Type specific protocol definitions.

## RTypeProtocol.hpp

Defines all packet types and data structures for the R-Type multiplayer protocol.

### Packet Types

- `CLIENT_HELLO` — connection request
- `CLIENT_INPUT` — player input (movement, shoot)
- `CLIENT_DISCONNECT` — clean disconnect
- `SERVER_WELCOME` — connection accepted
- `WORLD_SNAPSHOT` — full game state update (10 Hz)
- `ENTITY_SPAWN` — new entity created
- `ENTITY_DESTROY` — entity removed
- `PLAYER_DIED` — player death event

### Entity Types

- `ENTITY_PLAYER`
- `ENTITY_MONSTER`
- `ENTITY_PLAYER_MISSILE`
- `ENTITY_MONSTER_MISSILE`
- `ENTITY_OBSTACLE`
- `ENTITY_EXPLOSION`

### Key Structures

**ClientInput**: `playerId`, `inputMask` (bitfield for directional keys + shoot), `chargeLevel` (0–5)

**EntityState**: `id`, `type`, `position`, `velocity`, `hp`, `playerLine`, `chargeLevel`, `enemyType`, `projectileType`

## Shared Between

This header is included by both the server (`server/src/`) and the game client (`game/`, `client/`). It lives in `server/` because the server is authoritative and defines the protocol.
