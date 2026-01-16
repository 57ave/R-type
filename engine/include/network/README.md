# Engine Network Layer - Generic Components Only

This directory contains **ONLY generic, reusable networking code** that can be used by ANY game.

## 📦 What's Here (Generic)

- **`Packet.hpp`** - Generic packet structure with header (magic, version, type, seq, timestamp)
- **`UdpClient.hpp/cpp`** - Generic UDP client wrapper using ASIO
- **`UdpServer.hpp/cpp`** - Generic UDP server wrapper using ASIO
- **`ClientSession.hpp`** - Generic client session management (connection tracking, timeouts)
- **`Protocol.hpp`** - Generic protocol interface/utilities
- **`NetworkClient.hpp/cpp`** - Generic network client abstraction
- **`NetworkServer.hpp/cpp`** - Generic network server abstraction

These files have **NO knowledge of R-Type concepts** like players, enemies, missiles, charge shots, etc.

## ❌ What's NOT Here

**Game-specific protocols** like `RTypeProtocol.hpp` have been **moved to the game/server directories**.

Location: `server/include/network/RTypeProtocol.hpp`

This protocol defines R-Type specific packets:
- Entity types (PLAYER, MONSTER, MISSILE, etc.)
- Player-specific fields (playerId, chargeLevel, etc.)
- R-Type game logic packets (PLAYER_DIED, etc.)

## 🎯 Design Philosophy

**Engine = Generic**
- Packet structure: ✅ Generic
- UDP transport: ✅ Generic  
- Session management: ✅ Generic
- Protocol definition: ❌ Game-specific → Moved to game/server

**This allows:**
- Using the engine for ANY multiplayer game (FPS, racing, platformer, etc.)
- Each game defines its own protocol
- Maximum reusability

## 📝 Usage Example

### In Your Game

```cpp
// 1. Include generic engine networking
#include <network/UdpClient.hpp>
#include <network/Packet.hpp>

// 2. Include YOUR game-specific protocol
#include "network/MyGameProtocol.hpp"  // Your custom protocol!

// 3. Use generic transport with your custom packets
UdpClient client(io_context, "127.0.0.1", 12345);
MyGamePacket packet = createMyCustomPacket();
client.send(packet);
```

This keeps the engine generic while allowing game-specific networking! 🎮
