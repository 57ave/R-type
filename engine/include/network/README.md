# Engine Network Layer

This directory contains only generic, reusable networking code with no knowledge of R-Type game concepts.

## What's Here

- `Packet.hpp` — Generic packet structure (magic, version, type, seq, timestamp)
- `UdpClient.hpp/cpp` — UDP client wrapper using ASIO
- `UdpServer.hpp/cpp` — UDP server wrapper using ASIO
- `ClientSession.hpp` — Client session management (connection tracking, timeouts)
- `Protocol.hpp` — Generic protocol utilities
- `NetworkClient.hpp/cpp` — Network client abstraction
- `NetworkServer.hpp/cpp` — Network server abstraction

## What's NOT Here

Game-specific protocol definitions (`RTypeProtocol.hpp`) have been moved to `server/include/network/`. That file defines R-Type-specific packet types, entity types, and player fields.

## Design

The engine layer is game-agnostic. It handles transport only. Each game that uses this engine defines its own protocol on top of it.

```cpp
#include <network/UdpClient.hpp>
#include <network/Packet.hpp>
#include "network/RTypeProtocol.hpp"  // game-specific

UdpClient client(io_context, "127.0.0.1", 4242);
client.send(packet);
```
