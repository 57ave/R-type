# R-Type Network Module Documentation

This document describes the architecture and usage of the Network module located
in `engine/src/network` and `engine/include/network`.

## Architecture Overview

The network stack is built on top of **ASIO** (standalone) for asynchronous UDP
communication. It is designed to be:

- **Packet-based**: All communication relies on `NetworkPacket` structures.
- **Room-Architectured**: Clients connect to a Lobby and join specific Rooms to
  play.
- **Optimized**: Supports Serialization, Compression (RLE), and Quantization.

### Core Components

| Component         | File                | Description                                                                                      |
| ----------------- | ------------------- | ------------------------------------------------------------------------------------------------ |
| **NetworkServer** | `NetworkServer.hpp` | High-level server controller. Manages the `UdpServer` and `RoomManager`. Processes game packets. |
| **UdpServer**     | `UdpServer.hpp`     | Low-level UDP wrapper. Handles socket binding, async receive/send, and client session tracking.  |
| **RoomManager**   | `RoomManager.hpp`   | Manages creation, joining, and listing of game rooms (Lobbies).                                  |
| **ClientSession** | `ClientSession.hpp` | Represents a connected client (Endpoint, Last Packet Time, Room ID).                             |

---

## 2. Protocol & Data Definitions

### Packet Structure

Defined in `Packet.hpp`. All packets share a common header.

```cpp
struct PacketHeader {
    uint16_t magic;      // 0x5254 ('RT')
    uint8_t  version;    // Protocol Version (1)
    uint8_t  flags;      // Bit 0: Compressed
    uint16_t type;       // GamePacketType Enum
    uint32_t seq;        // Sequence Number
    uint32_t timestamp;  // Ms timestamp
};
```

### Protocol Types

Defined in `RTypeProtocol.hpp`.

| Packet Type      | ID   | Direction | Description                 |
| ---------------- | ---- | --------- | --------------------------- |
| `CLIENT_HELLO`   | 0x01 | C -> S    | Initial handshake.          |
| `SERVER_WELCOME` | 0x10 | S -> C    | Handshake response.         |
| `CLIENT_INPUT`   | 0x02 | C -> S    | Player input (keys).        |
| `WORLD_SNAPSHOT` | 0x11 | S -> C    | Complete game state update. |
| `CREATE_ROOM`    | 0x20 | C -> S    | Request to create a room.   |
| `JOIN_ROOM`      | 0x21 | C -> S    | Request to join a room.     |
| `ROOM_LIST`      | 0x22 | C -> S    | Request list of rooms.      |

### Serialization

We use a custom `Serializer` (`Serializer.hpp`) to write binary data safely.

- **Quantization**: `EntityState` positions (`x`, `y`) are stored as `int16_t`
  to save bandwidth (2 bytes vs 4 bytes).
- **Compression**: Payloads can be compressed using Run-Length Encoding (RLE)
  via `Compression.hpp`.

---

## 3. Usage Guide

### Starting the Server

```cpp
#include "network/NetworkServer.hpp"

// Start server on port 12345
NetworkServer server(12345);
server.start();

while (true) {
    // Process incoming packets (Room logic is auto-handled)
    server.process();

    // Consume Game Logic packets
    while (server.hasReceivedPackets()) {
        auto [packet, sender] = server.getNextReceivedPacket();
        if (packet.header.type == GamePacketType::CLIENT_INPUT) {
            // Handle Input
        }
    }
}
```

### Client-Side Prediction

Use `Prediction.hpp` to smooth movement on the client.

```cpp
// Interpolate between two snapshots for smooth rendering
auto renderState = Network::Prediction::interpolate(prevState, nextState, alphaVal);
```

---

## 4. Directory Structure

- **src/network/**
  - `NetworkServer.cpp`: Game/Lobby logic implementation.
  - `UdpServer.cpp`: Socket loop and session management.

- **include/network/**
  - `Serializer.hpp`: Binary Writer/Reader.
  - `Compression.hpp`: RLE algorithms.
  - `Room.hpp`: Room state definition.
  - `Packet.hpp`: Base Packet classes.
  - `RTypeProtocol.hpp`: Game-specific payload structs.
  - `Prediction.hpp`: Client interpolation math.
