/**
 * GamePackets.hpp - R-Type Network Packet Types
 * 
 * Defines all packet types exchanged between client and server
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <Packet.hpp>

namespace Network {

    /**
     * @brief Packet type IDs
     */
    enum class PacketType : uint16_t {
        // Connection
        CLIENT_CONNECT      = 1,    // Client requests connection
        SERVER_ACCEPT       = 2,    // Server accepts connection
        SERVER_REJECT       = 3,    // Server rejects connection
        CLIENT_DISCONNECT   = 4,    // Client disconnects
        PING                = 5,    // Heartbeat
        PONG                = 6,    // Heartbeat response
        
        // Lobby
        LOBBY_LIST_REQUEST  = 10,   // Request list of available rooms
        LOBBY_LIST_RESPONSE = 11,   // Server sends room list
        ROOM_CREATE         = 12,   // Create a new room
        ROOM_JOIN           = 13,   // Join existing room
        ROOM_LEAVE          = 14,   // Leave current room
        ROOM_UPDATE         = 15,   // Room state update (players, ready status)
        
        // Game
        GAME_START          = 20,   // Server signals game start
        PLAYER_INPUT        = 21,   // Client sends input
        ENTITY_SPAWN        = 22,   // Server spawns entity
        ENTITY_UPDATE       = 23,   // Server updates entity state
        ENTITY_DESTROY      = 24,   // Server destroys entity
        GAME_STATE          = 25,   // Full game state snapshot
        
        // Player
        PLAYER_READY        = 30,   // Player marks ready
        PLAYER_NOT_READY    = 31,   // Player marks not ready
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
