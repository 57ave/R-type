# Network Room Management Protocol

This document details the protocol and data structures used for managing game rooms in the R-Type network architecture.

## Overview

The R-Type server operates on a lobby-based system where players must join a "Room" before the game session begins.
- **Lobby**: The initial state where players can list, create, or join rooms.
- **Room**: A grouping of players who will play a game session together.

## Protocol Lifecycle

### 1. Listing Rooms
Clients request the list of available rooms to display in the lobby UI.

- **Request**: `ROOM_LIST` (0x22)
  - Payload: Empty.
- **Response**: `ROOM_LIST_REPLY` (0x31)
  - Payload: `RoomListPayload`
    - `uint32 count`: Number of rooms.
    - `RoomInfo[] rooms`: Array of room details.

### 2. Creating a Room
A player can create a new room, becoming its host.

- **Request**: `CREATE_ROOM` (0x20)
  - Payload: `CreateRoomPayload`
    - `string name`: Name of the room.
    - `uint8 maxPlayers`: Maximum number of players allowed.
- **Response**: `ROOM_CREATED` (0x32)
  - Payload: `uint32 roomId`: The ID of the newly created room.
  - *Note*: The creator implicitly joins the room and receives a `ROOM_JOINED` packet immediately after.

### 3. Joining a Room
Players join an existing room using its ID.

- **Request**: `JOIN_ROOM` (0x21)
  - Payload: `JoinRoomPayload`
    - `uint32 roomId`: ID of the room to join.
- **Response**: `ROOM_JOINED` (0x30)
  - Payload: `RoomJoinedPayload`
    - `uint32 roomId`: ID of the joined room.
    - `string roomName`: Name of the room.
    - `uint8 maxPlayers`: Max capacity.
    - `uint32 hostPlayerId`: Network ID of the room host.

### 4. Room Updates
Once inside a room, clients need to know about other players joining, leaving, or changing state (ready/host).

- **Event**: `ROOM_PLAYERS_UPDATE` (0x33)
  - Direction: Server -> Client
  - Triggered when: A player joins, leaves, or changes readiness.
  - Payload: `RoomPlayersPayload`
    - `uint32 roomId`: Current room ID.
    - `uint32 count`: Number of players.
    - `PlayerInRoomInfo[] players`: List of players.

### 5. Game Start
The host can initiate the game when players are ready.

- **Request**: `GAME_START` (0x23)
  - Payload: Empty.
  - *Condition*: Only the host can send this.
- **Broadcast**: `GAME_START` (0x23)
  - Direction: Server -> All Clients in Room.
  - Action: Clients transition from Lobby state to Playing state.

## Data Structures

### RoomInfo
Used in `ROOM_LIST_REPLY`.
```cpp
struct RoomInfo {
    uint32_t id;
    std::string name;
    uint8_t currentPlayers;
    uint8_t maxPlayers;
};
```

### PlayerInRoomInfo
Used in `ROOM_PLAYERS_UPDATE`.
```cpp
struct PlayerInRoomInfo {
    uint32_t playerId;
    std::string playerName;
    bool isHost;
    bool isReady;
};
```

## Packet Type Reference

| Packet Name | ID | Description |
| :--- | :--- | :--- |
| `CREATE_ROOM` | 0x20 | Client requests to create room |
| `JOIN_ROOM` | 0x21 | Client requests to join room |
| `ROOM_LIST` | 0x22 | Client requests list of rooms |
| `GAME_START` | 0x23 | Host starts the game |
| `ROOM_JOINED` | 0x30 | Server confirms room join |
| `ROOM_LIST_REPLY` | 0x31 | Server sends room list |
| `ROOM_CREATED` | 0x32 | Server confirms room creation |
| `ROOM_PLAYERS_UPDATE` | 0x33 | Server updates player list |
| `CHAT_MESSAGE` | 0x40 | Chat communication within room |
