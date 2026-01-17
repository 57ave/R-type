#pragma once

#include <network/Packet.hpp>
#include <array>



struct CreateRoomPayload {
    std::string name;
    uint8_t maxPlayers;

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.writeString(name);
        serializer.write(maxPlayers);
        return serializer.getBuffer();
    }

    static CreateRoomPayload deserialize(std::vector<char> data) { // Pass by value or const ref
        Network::Deserializer deserializer(data);
        CreateRoomPayload payload;
        payload.name = deserializer.readString();
        payload.maxPlayers = deserializer.read<uint8_t>();
        return payload;
    }
};

struct JoinRoomPayload {
    uint32_t roomId;

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(roomId);
        return serializer.getBuffer();
    }

    static JoinRoomPayload deserialize(const std::vector<char>& data) {
        Network::Deserializer deserializer(data);
        JoinRoomPayload payload;
        payload.roomId = deserializer.read<uint32_t>();
        return payload;
    }
};

struct RoomInfo {
    uint32_t id;
    std::string name;
    uint8_t currentPlayers;
    uint8_t maxPlayers;

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(id);
        serializer.writeString(name);
        serializer.write(currentPlayers);
        serializer.write(maxPlayers);
        return serializer.getBuffer();
    }
   
   static RoomInfo deserialize(Network::Deserializer& deserializer) {
       RoomInfo info;
       info.id = deserializer.read<uint32_t>();
       info.name = deserializer.readString();
       info.currentPlayers = deserializer.read<uint8_t>();
       info.maxPlayers = deserializer.read<uint8_t>();
       return info;
   }
};

struct RoomListPayload {
    std::vector<RoomInfo> rooms;

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(static_cast<uint32_t>(rooms.size()));
        for(const auto& room : rooms) {
            auto buf = room.serialize();
            serializer.writeBytes(buf.data(), buf.size());
        }
        return serializer.getBuffer();
    }
    
    static RoomListPayload deserialize(const std::vector<char>& data) {
         Network::Deserializer deserializer(data);
         RoomListPayload payload;
         uint32_t count = deserializer.read<uint32_t>();
         for(uint32_t i=0; i<count; ++i) {
             payload.rooms.push_back(RoomInfo::deserialize(deserializer));
         }
         return payload;
     }
};

struct RenameRoomPayload {
    uint32_t roomId;
    std::string newName;

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(roomId);
        serializer.writeString(newName);
        return serializer.getBuffer();
    }

    static RenameRoomPayload deserialize(const std::vector<char>& data) {
        Network::Deserializer deserializer(data);
        RenameRoomPayload payload;
        payload.roomId = deserializer.read<uint32_t>();
        payload.newName = deserializer.readString();
        return payload;
    }
};


enum class GamePacketType : uint16_t {
    CLIENT_HELLO = 0x01,
    CLIENT_INPUT = 0x02,
    CLIENT_PING = 0x03,
    CLIENT_DISCONNECT = 0x04,
    CREATE_ROOM = 0x20,
    JOIN_ROOM = 0x21,
    ROOM_LIST = 0x22,
    GAME_START = 0x23,
    RENAME_ROOM = 0x24,
    SERVER_WELCOME = 0x10,
    WORLD_SNAPSHOT = 0x11,
    ENTITY_SPAWN = 0x12,
    ENTITY_DESTROY = 0x13,
    PLAYER_DIED = 0x14,
    SERVER_PING_REPLY = 0x15,
    CLIENT_LEFT = 0x16,
    ROOM_JOINED = 0x30,
    ROOM_LIST_REPLY = 0x31,
    ROOM_CREATED = 0x32
};

// Enum for EntityType
enum class EntityType : uint8_t {
    ENTITY_PLAYER = 0,
    ENTITY_MONSTER = 1,
    ENTITY_PLAYER_MISSILE = 2,
    ENTITY_MONSTER_MISSILE = 3,
    ENTITY_OBSTACLE = 4,
    ENTITY_EXPLOSION = 5
};

#pragma pack(push, 1)

// ClientInput payload
struct ClientInput {
    uint8_t playerId;
    uint8_t inputMask;
    uint8_t chargeLevel; // 0 = normal shot, 1-5 = charge levels

    ClientInput() : playerId(0), inputMask(0), chargeLevel(0) {}

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    static ClientInput deserialize(const char* data) {
        Network::Deserializer deserializer(data, sizeof(ClientInput));
        return deserializer.read<ClientInput>();
    }
};

// SnapshotHeader
struct SnapshotHeader {
    uint32_t entityCount;

    SnapshotHeader() : entityCount(0) {}

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    static SnapshotHeader deserialize(const char* data) {
         Network::Deserializer deserializer(data, sizeof(SnapshotHeader));
         return deserializer.read<SnapshotHeader>();
    }
};

// EntityState
struct EntityState {
    uint32_t id;
    EntityType type;
    int16_t x;  // Quantized position (pixel coordinates)
    int16_t y;
    int16_t vx; // Quantized velocity
    int16_t vy;
    uint8_t hp;
    uint8_t playerLine; // Pour la couleur du vaisseau (ligne dans la spritesheet)
    
    // Extended fields for variety
    uint8_t chargeLevel;    // For missiles (0 = normal, 1-5 = charge levels)
    uint8_t enemyType;      // For enemies (0 = basic, 1 = zigzag, etc.)
    uint8_t projectileType; // For projectiles (0 = normal, 1 = charged, etc.)

    EntityState() : id(0), type(EntityType::ENTITY_PLAYER), x(0), y(0), vx(0), vy(0), hp(0), playerLine(0), 
                    chargeLevel(0), enemyType(0), projectileType(0) {}

    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    static EntityState deserialize(const char* data) {
        Network::Deserializer deserializer(data, sizeof(EntityState));
        return deserializer.read<EntityState>();
    }
};

#pragma pack(pop)

struct RTypeProtocol {
    
    static NetworkPacket createClientInputPacket(const ClientInput& input) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CLIENT_INPUT));
        packet.setPayload(input.serialize());
        return packet;
    }

    static ClientInput getClientInput(const NetworkPacket& packet) {
        if (packet.header.type != static_cast<uint16_t>(GamePacketType::CLIENT_INPUT)) {
             throw std::runtime_error("Invalid packet type for CLIENT_INPUT");
        }
        Network::Deserializer deserializer(packet.payload);
        return deserializer.read<ClientInput>();
    }

    static NetworkPacket createWorldSnapshotPacket(const SnapshotHeader& snapHeader, const std::vector<EntityState>& entities) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT));
        Network::Serializer serializer;
        serializer.write(snapHeader);
        for (const auto& entity : entities) {
            serializer.write(entity);
        }
        packet.setPayload(serializer.getBuffer());
        return packet;
    }

    static std::pair<SnapshotHeader, std::vector<EntityState>> getWorldSnapshot(const NetworkPacket& packet) {
        if (packet.header.type != static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT)) {
            throw std::runtime_error("Invalid packet type for WORLD_SNAPSHOT");
        }
        
        Network::Deserializer deserializer(packet.payload);
        SnapshotHeader snapHeader = deserializer.read<SnapshotHeader>();
        std::vector<EntityState> entities;
        entities.reserve(snapHeader.entityCount);
        
        for (uint32_t i = 0; i < snapHeader.entityCount; ++i) {
            entities.push_back(deserializer.read<EntityState>());
        }
        return {snapHeader, entities};
    }
};
