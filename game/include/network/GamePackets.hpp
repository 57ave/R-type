/**
 * GamePackets.hpp - R-Type Network Packet Types
 * 
 * Defines all packet types exchanged between client and server
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <network/Packet.hpp>

namespace Network {

    /**
     * @brief Packet type IDs
     */
    enum class PacketType : uint16_t {
        // Connection - Compatible with GamePacketType
        CLIENT_CONNECT      = 0x01, // CLIENT_HELLO - Client requests connection
        SERVER_ACCEPT       = 0x10, // SERVER_WELCOME - Server accepts connection
        SERVER_REJECT       = 0x03, // Server rejects connection
        CLIENT_DISCONNECT   = 0x04, // Client disconnects
        PING                = 0x03, // CLIENT_PING - Heartbeat
        PONG                = 0x15, // SERVER_PING_REPLY - Heartbeat response
        
        // Lobby - Compatible with GamePacketType
        LOBBY_LIST_REQUEST  = 0x22, // ROOM_LIST - Request list of available rooms
        LOBBY_LIST_RESPONSE = 0x31, // ROOM_LIST_REPLY - Server sends room list
        ROOM_CREATE         = 0x20, // CREATE_ROOM - Create a new room
        ROOM_CREATED        = 0x32, // ROOM_CREATED - Server confirms room creation
        ROOM_JOIN           = 0x21, // JOIN_ROOM - Join existing room
        ROOM_JOINED         = 0x30, // ROOM_JOINED - Server confirms room join
        ROOM_LEAVE          = 0x25, // ROOM_LEAVE - Leave current room
        ROOM_UPDATE         = 0x33, // ROOM_PLAYERS_UPDATE - Room state update
        
        // Game - Compatible with GamePacketType
        GAME_START          = 0x23, // GAME_START - Server signals game start
        PLAYER_INPUT        = 0x02, // CLIENT_INPUT - Client sends input
        ENTITY_SPAWN        = 0x12, // ENTITY_SPAWN - Server spawns entity
        ENTITY_UPDATE       = 0x11, // WORLD_SNAPSHOT - Server updates entity state
        ENTITY_DESTROY      = 0x13, // ENTITY_DESTROY - Server destroys entity
        GAME_STATE          = 0x11, // WORLD_SNAPSHOT - Full game state snapshot
        
        // Player (no direct equivalent, using custom IDs)
        PLAYER_READY        = 0x50, // Player marks ready
        PLAYER_NOT_READY    = 0x51, // Player marks not ready
        
        // Level system
        LEVEL_CHANGE        = 0x60, // Server signals level change
        
        // Game end conditions
        GAME_OVER           = 0x70, // Server signals all players dead (payload: uint32_t totalScore)
        GAME_VICTORY        = 0x71, // Server signals boss L3 killed (payload: uint32_t totalScore)
        
        // Chat
        CHAT_MESSAGE        = 0x40, // Chat message (inter-room)
    };

    /**
     * @brief Client connection request
     */
    struct ClientConnectPacket {
        char playerName[32];
        uint32_t version;
        
        static NetworkPacket serialize(const std::string& name, uint32_t ver = 1) {
            NetworkPacket packet(static_cast<uint16_t>(PacketType::CLIENT_CONNECT));
            ClientConnectPacket data;
            std::strncpy(data.playerName, name.c_str(), sizeof(data.playerName) - 1);
            data.playerName[sizeof(data.playerName) - 1] = '\0';
            data.version = ver;
            
            packet.payload.resize(sizeof(data));
            std::memcpy(packet.payload.data(), &data, sizeof(data));
            return packet;
        }
        
        static ClientConnectPacket deserialize(const NetworkPacket& packet) {
            ClientConnectPacket data;
            std::memcpy(&data, packet.payload.data(), sizeof(data));
            return data;
        }
    };

    /**
     * @brief Server accept/reject response
     */
    struct ServerResponsePacket {
        uint32_t clientId;
        bool accepted;
        char message[128];
        
        static NetworkPacket serialize(uint32_t id, bool accept, const std::string& msg = "") {
            NetworkPacket packet(static_cast<uint16_t>(
                accept ? PacketType::SERVER_ACCEPT : PacketType::SERVER_REJECT
            ));
            ServerResponsePacket data;
            data.clientId = id;
            data.accepted = accept;
            std::strncpy(data.message, msg.c_str(), sizeof(data.message) - 1);
            data.message[sizeof(data.message) - 1] = '\0';
            
            packet.payload.resize(sizeof(data));
            std::memcpy(packet.payload.data(), &data, sizeof(data));
            return packet;
        }
        
        static ServerResponsePacket deserialize(const NetworkPacket& packet) {
            ServerResponsePacket data;
            std::memcpy(&data, packet.payload.data(), sizeof(data));
            return data;
        }
    };

    /**
     * @brief Room information
     */
    struct RoomInfo {
        uint32_t roomId;
        char roomName[64];
        uint8_t currentPlayers;
        uint8_t maxPlayers;
        bool inGame;
    };

    /**
     * @brief Lobby list response
     */
    struct LobbyListPacket {
        uint32_t roomCount;
        RoomInfo rooms[16];  // Max 16 rooms for simplicity
        
        static NetworkPacket serialize(const std::vector<RoomInfo>& roomList) {
            NetworkPacket packet(static_cast<uint16_t>(PacketType::LOBBY_LIST_RESPONSE));
            LobbyListPacket data;
            data.roomCount = std::min(static_cast<uint32_t>(roomList.size()), 16u);
            
            for (size_t i = 0; i < data.roomCount; ++i) {
                data.rooms[i] = roomList[i];
            }
            
            packet.payload.resize(sizeof(data));
            std::memcpy(packet.payload.data(), &data, sizeof(data));
            return packet;
        }
        
        static LobbyListPacket deserialize(const NetworkPacket& packet) {
            LobbyListPacket data;
            std::memcpy(&data, packet.payload.data(), sizeof(data));
            return data;
        }
    };

    /**
     * @brief Room creation/join request
     */
    struct RoomActionPacket {
        char roomName[64];
        uint8_t maxPlayers;
        
        static NetworkPacket serialize(PacketType type, const std::string& name, uint8_t max = 4) {
            NetworkPacket packet(static_cast<uint16_t>(type));
            RoomActionPacket data;
            std::strncpy(data.roomName, name.c_str(), sizeof(data.roomName) - 1);
            data.roomName[sizeof(data.roomName) - 1] = '\0';
            data.maxPlayers = max;
            
            packet.payload.resize(sizeof(data));
            std::memcpy(packet.payload.data(), &data, sizeof(data));
            return packet;
        }
        
        static RoomActionPacket deserialize(const NetworkPacket& packet) {
            RoomActionPacket data;
            std::memcpy(&data, packet.payload.data(), sizeof(data));
            return data;
        }
    };

} // namespace Network
