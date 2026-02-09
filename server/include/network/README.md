# R-Type Network Protocol

This directory contains **R-Type specific network protocol definitions**.

## 📄 RTypeProtocol.hpp

Defines the R-Type multiplayer protocol:

### Packet Types
- `CLIENT_HELLO` - Client connection request
- `CLIENT_INPUT` - Player input (movement, shoot, bomb)
- `CLIENT_DISCONNECT` - Client leaving
- `SERVER_WELCOME` - Server accepting client
- `WORLD_SNAPSHOT` - Full game state update
- `ENTITY_SPAWN` - New entity created
- `ENTITY_DESTROY` - Entity destroyed
- `PLAYER_DIED` - Player death event

### Entity Types
- `ENTITY_PLAYER` - Player ship
- `ENTITY_MONSTER` - Enemy ship
- `ENTITY_PLAYER_MISSILE` - Player projectile
- `ENTITY_MONSTER_MISSILE` - Enemy projectile
- `ENTITY_OBSTACLE` - Destructible obstacle
- `ENTITY_EXPLOSION` - Visual effect

### Data Structures

**ClientInput**
- `playerId` - Player identifier
- `inputMask` - Bitfield for arrow keys, shoot, bomb
- `chargeLevel` - Charge shot level (0-5)

**EntityState**
- Standard fields: id, type, position, velocity, hp
- R-Type specific:
  - `playerLine` - Player color/spritesheet row
  - `chargeLevel` - Missile charge level
  - `enemyType` - Enemy behavior type
  - `projectileType` - Projectile visual type

## 🔗 Shared Between

- **Server** (`server/src/`) - Authoritative game server
- **Client** (`game/` and `client/`) - Game clients

The protocol is defined here (in server) as the **server is authoritative**.
Clients and game code include it via CMake include paths.

## 🎮 Why Here?

This protocol is **100% R-Type specific**. Moving it out of the engine allows:
- Engine to be reusable for other games
- Clear separation of generic vs game-specific code
- Each game can define its own protocol

Other games using the engine would create their own protocol definitions!
