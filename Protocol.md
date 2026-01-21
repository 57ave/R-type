R-Type Binary UDP Protocol
=================================

RFC: Draft - R-Type Binary UDP Protocol
Author: R-Type Project
Date: 2026-01-21
Version: 1.1 (implementation default +network optimisation)

Abstract
--------
This document defines the binary wire format used by the R-Type project for UDP-based client/server communication. It describes the packet header, packet types, payload encodings, serialization rules and recommended validation behavior. The specification is intended for implementers who need to interoperate with the existing engine and server code (see `engine/include/network/Packet.hpp`, `engine/include/network/Serializer.hpp`, `server/include/network/RTypeProtocol.hpp`).

Status of This Memo
-------------------
This memo provides a project-specific protocol definition. It is not an IETF standard; the document uses RFC-style structure and RFC 2119 key words (MUST, MUST NOT, SHOULD, MAY) for clarity.

Conventions and Terminology
---------------------------
The key words MUST, MUST NOT, REQUIRED, SHOULD, SHOULD NOT, RECOMMENDED, MAY, and OPTIONAL in this document are to be interpreted as described in RFC 2119.

Terminology
* "Host" and "Server" refer to the authoritative game server process.
* "Client" refers to a game client implementation.
* "Packet" is a single UDP datagram carrying a `PacketHeader` and an optional payload.

Table of Contents
-----------------
1. Overview
2. Packet Header (wire layout)
3. Packet Type Registry
4. Payload Specifications
   4.1. ClientInput
   4.2. SnapshotHeader and EntityState
   4.3. Room and Lobby payloads
   4.4. Chat payload
5. Serialization Rules and Encodings
6. Sequence, Timestamp and Reliability Notes
7. Error Handling and Validation (MUST requirements)
8. Security Considerations
9. Examples
10. IANA Considerations
11. References

1. Overview
-----------
This protocol uses UDP exclusively for real-time game traffic. The wire format is compact, binary and oriented towards low-latency game state updates. Implementations MUST follow the exact binary field sizes and encodings described in this document to interoperate with the existing server implementation.

2. Packet Header (wire layout)
-----------------------------
All UDP packets start with a fixed-size header called `PacketHeader`. The header is serialized in little-endian byte order and contains the following fields (byte offsets shown as decimal, size in bytes):

   0:  magic        (uint16)  2 bytes  ; fixed signature (0x5254)
   2:  version      (uint8)   1 byte   ; protocol version (default 1)
   3:  flags        (uint8)   1 byte   ; flag bits (bit 0 = compressed)
   4:  type         (uint16)  2 bytes  ; packet type identifier (see Section 3)
   6:  seq          (uint32)  4 bytes  ; sequence number
  10:  timestamp    (uint32)  4 bytes  ; timestamp in milliseconds since server start

Header total size: 14 bytes.

Field semantics
* `magic` MUST be 0x5254 (ASCII 'RT' in big-endian view). Implementations SHOULD verify this field and discard packets with an unexpected magic value.
* `version` indicates the protocol version. If a receiver does not understand the version, it SHOULD drop the packet and MAY log the event.
* `flags` is an 8-bit flags field. Bit 0 is reserved to signal compressed payloads; other bits are reserved for future use. Receivers MUST ignore unknown flag bits.
* `type` is a 16-bit identifier that selects how the payload is interpreted (see Section 3).
* `seq` is a monotonic sequence number assigned by the sender to help detect duplicates and reordering.
* `timestamp` contains the sender's notion of time in milliseconds (server start epoch). It is advisory and MAY be used for smoothing/latency estimation.

Serialization note: The header is serialized before the payload. The receiver MUST parse the first 14 bytes as the header and then decode the rest as payload bytes according to `type`.

3. Packet Type Registry
-----------------------
Packet type identifiers are defined in `server/include/network/RTypeProtocol.hpp` (type: uint16). The following registry documents the values used in the current implementation.

| Value  | Name                      | Direction        | Purpose |
|--------|---------------------------|------------------|---------|
| 0x0001 | CLIENT_HELLO              | Client → Server  | Initial connection / hello |
| 0x0002 | CLIENT_INPUT              | Client → Server  | Player input payload (see 4.1) |
| 0x0003 | CLIENT_PING               | Client → Server  | Keep-alive / ping |
| 0x0004 | CLIENT_DISCONNECT         | Client → Server  | Graceful disconnect |
| 0x0020 | CREATE_ROOM               | Client → Server  | Create a lobby/room (payload: name + maxPlayers) |
| 0x0021 | JOIN_ROOM                 | Client → Server  | Join a room (payload: roomId) |
| 0x0022 | ROOM_LIST                 | Client → Server  | Request room list |
| 0x0023 | GAME_START                | Client → Server  | Request start (host) |
| 0x0024 | RENAME_ROOM               | Client → Server  | Rename a room |
| 0x0030 | ROOM_JOINED               | Server → Client  | Confirmation that join succeeded |
| 0x0031 | ROOM_LIST_REPLY           | Server → Client  | Reply to ROOM_LIST, contains multiple RoomInfo |
| 0x0032 | ROOM_CREATED              | Server → Client  | Reply to CREATE_ROOM, contains roomId |
| 0x0033 | ROOM_PLAYERS_UPDATE       | Server → Client  | Update player list for a room |
| 0x0034 | CLIENT_TOGGLE_PAUSE       | Client → Server  | Host toggles pause for room |
| 0x0035 | SERVER_SET_PAUSE          | Server → Client  | Server informs clients of paused state |
| 0x0040 | CHAT_MESSAGE              | Bidirectional    | Chat messages (see 4.4) |
| 0x0010 | SERVER_WELCOME            | Server → Client  | Welcome / assign player id |
| 0x0011 | WORLD_SNAPSHOT           | Server → Client  | Snapshot of world state (see 4.2) |
| 0x0012 | ENTITY_SPAWN              | Server → Client  | Notify client about a new entity |
| 0x0013 | ENTITY_DESTROY            | Server → Client  | Notify client an entity was destroyed |
| 0x0014 | PLAYER_DIED               | Server → Client  | Notify client that a player died |
| 0x0015 | SERVER_PING_REPLY         | Server → Client  | Ping reply |
| 0x0016 | CLIENT_LEFT               | Server → Client  | Tell clients a player left |

Note: Values are expressed as 16-bit unsigned integers and are encoded in little-endian on the wire.

4. Payload Specifications
-------------------------
All payload encodings follow the `Network::Serializer` and `Network::Deserializer` semantics (see Section 5). For POD structures that are written with `write(const T&)`, the serialized representation is the raw bytes of the packed object in little-endian order.

4.1 ClientInput (CLIENT_INPUT, 0x0002)
-------------------------------------
When `type` == CLIENT_INPUT the payload is a packed POD with the following layout (wire order and sizes):

   Offset  Size  Name         Type    Description
   0       1     playerId     uint8   Player id assigned by server (0..255)
   1       1     inputMask    uint8   Bitmask of inputs (see below)
   2       1     chargeLevel  uint8   0 = normal shot, 1-5 = charge levels

Total payload size: 3 bytes.

Input mask bits (LSB = bit 0):
* Bit 0 = Move Up
* Bit 1 = Move Down
* Bit 2 = Move Left
* Bit 3 = Move Right
* Bit 4 = Fire
* Bits 5..7 = reserved (MUST be zero when sending; receivers MUST ignore them)

4.2 SnapshotHeader and EntityState (WORLD_SNAPSHOT, 0x0011)
----------------------------------------------------------
A WORLD_SNAPSHOT payload begins with a `SnapshotHeader` followed by `entityCount` consecutive `EntityState` records.

SnapshotHeader (wire):
   Offset  Size  Name         Type    Description
   0       4     entityCount  uint32  Number of EntityState entries (little-endian)

EntityState (packed POD) layout (wire):
   Offset Size Name            Type      Description
    0     4    id              uint32    Entity unique id
    4     1    type            uint8     EntityType (see Section 4.2.1)
    5     2    x               int16     Quantized X position (pixels)
    7     2    y               int16     Quantized Y position (pixels)
    9     2    vx              int16     Quantized X velocity
   11     2    vy              int16     Quantized Y velocity
   13     1    hp              uint8     Hit points (0 = dead)
   14     1    playerLine      uint8     Sprite row / player color index
   15     1    playerId        uint8     Owner player id (0 = none)
   16     1    chargeLevel     uint8     For missiles: 0 = normal, 1-5 = charged
   17     1    enemyType       uint8     Enemy subtype (implementation-defined)
   18     1    projectileType  uint8     Projectile subtype

Total EntityState size: 19 bytes. The snapshot payload length MUST be 4 + entityCount * 19 bytes.

4.2.1 EntityType

EntityType is a uint8 enumerated as follows (values from `RTypeProtocol.hpp`):
* 0 = ENTITY_PLAYER
* 1 = ENTITY_MONSTER
* 2 = ENTITY_PLAYER_MISSILE
* 3 = ENTITY_MONSTER_MISSILE
* 4 = ENTITY_OBSTACLE
* 5 = ENTITY_EXPLOSION

4.3 Room and Lobby payloads
---------------------------
Several packets use variable-length encodings with strings and nested lists. These payloads are serialized using the `Network::Serializer` helpers and follow the same wire rules described in Section 5. The most important types are:

4.3.1 CreateRoomPayload (CREATE_ROOM, 0x0020)
   - name: string
   - maxPlayers: uint8

Wire: `writeString(name)` then `write(maxPlayers)`.

4.3.2 JoinRoomPayload (JOIN_ROOM, 0x0021)
   - roomId: uint32

Wire: 4-byte roomId (LE).

4.3.3 RoomInfo and RoomListPayload (ROOM_LIST_REPLY, 0x0031)
RoomInfo fields:
   - id: uint32
   - name: string
   - currentPlayers: uint8
   - maxPlayers: uint8

RoomListPayload wire format: 32-bit count N, followed by N RoomInfo entries. Each RoomInfo is serialized by writing id (uint32), then `writeString(name)`, then currentPlayers and maxPlayers as uint8s.

4.3.4 PlayerInRoomInfo and RoomPlayersPayload (ROOM_PLAYERS_UPDATE, 0x0033)
PlayerInRoomInfo fields:
   - playerId: uint32
   - playerName: string
   - isHost: bool (stored as 1 byte)
   - isReady: bool (1 byte)

RoomPlayersPayload wire: roomId (uint32), count (uint32), then for each player the serialized PlayerInRoomInfo.

4.3.5 RenameRoomPayload (RENAME_ROOM, 0x0024)
   - roomId: uint32, newName: string

4.4 ChatMessagePayload (CHAT_MESSAGE, 0x0040)

Fields:
   - senderId: uint32
   - senderName: string
   - message: string
   - roomId: uint32

Wire: senderId (LE), writeString(senderName), writeString(message), roomId (LE).

5. Serialization Rules and Encodings
-----------------------------------
5.1 Endianness
All integer-valued fields are encoded in little-endian byte order. Implementations MUST use little-endian for all integer fields to interoperate with the current server implementation.

5.2 POD writing
POD structures that are written using `Network::Serializer::write(const T&)` are copied as raw bytes (memcpy) of the in-memory representation of the packed struct. Therefore:
* Structures MUST be declared with packing to avoid padding (the implementation uses `#pragma pack(push, 1)` around packed PODs).
* Only types satisfying `std::is_standard_layout<T>::value` SHOULD be written as PODs with `write(T)`.

5.3 Strings
Strings are encoded as a 32-bit unsigned integer length (uint32 little-endian), followed by exactly `length` bytes of UTF-8 encoded data. An empty string is encoded as length=0 and no subsequent bytes.

5.4 Booleans
Booleans serialized by `write(bool)` are encoded as a single byte: 0x00 for false, 0x01 for true. Receivers MUST accept any non-zero byte as true (to be liberal in acceptance) but implementations SHOULD send exactly 0x00 or 0x01.

5.5 Payload framing
A `NetworkPacket` on the wire is the header followed by payload bytes. The receiver MUST verify that the total UDP datagram length is at least 14 bytes (header size). The remaining bytes (datagram_len - 14) are the payload length. Each packet type defines its required payload shape and size; the receiver MUST validate that the payload length matches expected sizes for fixed-size payloads or that the `Deserializer` operations do not underflow for variable-length payloads (strings/lists).

6. Sequence, Timestamp and Reliability Notes
-------------------------------------------
* Sequence (`seq`) is used to detect duplicates, reorder and drop late packets. Receivers SHOULD ignore packets with sequence numbers older than their current acknowledged sequence window.
* Timestamp is advisory and may be used by clients for interpolation and latency estimation. Clocks are not synchronized; timestamp values are measured from server start and are not absolute wall-clock time.
* UDP is used for low-latency streaming. The protocol implements application-level measures (sequence numbers, periodic keepalive pings) rather than reliable, in-order delivery for every message.

7. Error Handling and Validation (MUST requirements)
--------------------------------------------------
Receivers MUST:
* Verify the packet contains at least the 14-byte header; otherwise discard the packet.
* Verify `magic` equals 0x5254; otherwise discard the packet.
* If `version` is unsupported, drop and optionally log the packet.
* For fixed-size payloads (e.g., `ClientInput`, `EntityState`), verify the payload length is exactly the expected number of bytes and discard otherwise.
* For variable-length payloads, use the `Deserializer` and handle buffer underflow errors by discarding the packet; underflow MUST NOT crash the process.
* Ignore unknown packet `type` values (log as appropriate), but MUST NOT crash.

Senders SHOULD:
* Set reserved bits and fields to zero.
* Avoid sending excessively large strings; servers MAY impose maximum lengths and disconnect clients that exceed sane thresholds.

8. Security Considerations
--------------------------
* The protocol accepts untrusted input from the network. Implementations MUST validate every length field before allocating memory to avoid uncontrolled memory allocations or buffer overruns.
* All deserialization operations MUST be bounds-checked. The existing `Network::Deserializer` throws on buffer underflow; server code MUST catch these exceptions and recover gracefully.
* Implementations SHOULD rate-limit resource-intensive operations triggered by client packets (e.g., large room lists, expensive name operations) to mitigate abusive clients.
* The `magic` value prevents accidental processing of unrelated UDP traffic but is not a security measure. Do not rely on magic as authentication.
* Consider integrating an authenticated handshake (out of scope for this document) over TCP or a secure UDP layer if stronger authentication or anti-spoofing is required.

9. Examples
-----------
Example 1: Simple CLIENT_INPUT packet (playerId=1, inputMask=0x04 (Move Left), chargeLevel=0)

Header fields chosen for the example:
* magic = 0x5254
* version = 1
* flags = 0x00
* type = CLIENT_INPUT (0x0002)
* seq = 0x0000000A (10)
* timestamp = 0x000003E8 (1000 ms)

Wire bytes (hex, little-endian):

54 52 01 00 02 00 0A 00 00 00 E8 03 00 00 01 04 00

Breakdown:
* 54 52        = magic (0x5254) (LE representation: 0x54 0x52)
* 01           = version
* 00           = flags
* 02 00        = type (0x0002) CLIENT_INPUT
* 0A 00 00 00  = seq (10)
* E8 03 00 00  = timestamp (1000)
* 01 04 00     = payload (playerId=1, inputMask=0x04, chargeLevel=0)

Example 2: WORLD_SNAPSHOT with two entities (conceptual)
* SnapshotHeader.entityCount = 2 (00 00 00 02)
* Followed by two EntityState records, each 19 bytes as specified in 4.2.

(For brevity the full hex blob is omitted; implementers can build the serializer using the helper functions in `RTypeProtocol.hpp`.)
