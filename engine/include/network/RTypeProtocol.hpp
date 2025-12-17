#pragma once

#include "Packet.hpp"
#include <array>


// Enum for PacketType
// We use uint16_t in the header, so this maps directly.
enum class GamePacketType : uint16_t {
    CLIENT_HELLO = 0x01,
    CLIENT_INPUT = 0x02,
    CLIENT_PING = 0x03,
    CLIENT_DISCONNECT = 0x04,
    SERVER_WELCOME = 0x10,
    WORLD_SNAPSHOT = 0x11,
    ENTITY_SPAWN = 0x12,
    ENTITY_DESTROY = 0x13,
    PLAYER_DIED = 0x14,
    SERVER_PING_REPLY = 0x15,
    CLIENT_LEFT = 0x16
};

// Enum for EntityType
enum class EntityType : uint8_t {
    ENTITY_PLAYER = 0,
    ENTITY_MONSTER = 1,
    ENTITY_PLAYER_MISSILE = 2,
    ENTITY_MONSTER_MISSILE = 3,
    ENTITY_OBSTACLE = 4
};

#pragma pack(push, 1)

// ClientInput payload
struct ClientInput {
    uint8_t playerId;
    uint8_t inputMask;

    ClientInput() : playerId(0), inputMask(0) {}

    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    static ClientInput deserialize(const char* data) {
        ClientInput input;
        std::memcpy(&input, data, sizeof(input));
        return input;
    }
};

// SnapshotHeader
struct SnapshotHeader {
    uint32_t entityCount;

    SnapshotHeader() : entityCount(0) {}

    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    static SnapshotHeader deserialize(const char* data) {
        SnapshotHeader header;
        std::memcpy(&header, data, sizeof(header));
        return header;
    }
};

// EntityState
struct EntityState {
    uint32_t id;
    EntityType type;
    float x;
    float y;
    float vx;
    float vy;
    uint8_t hp;
    uint8_t playerLine; // Pour la couleur du vaisseau (ligne dans la spritesheet)

    EntityState() : id(0), type(EntityType::ENTITY_PLAYER), x(0.0f), y(0.0f), vx(0.0f), vy(0.0f), hp(0), playerLine(0) {}

    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    static EntityState deserialize(const char* data) {
        EntityState state;
        std::memcpy(&state, data, sizeof(state));
        return state;
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
        if (packet.header.type != static_cast<uint16_t>(GamePacketType::CLIENT_INPUT) || packet.payload.size() != sizeof(ClientInput)) {
            throw std::runtime_error("Invalid payload for CLIENT_INPUT");
        }
        return ClientInput::deserialize(packet.payload.data());
    }

    static NetworkPacket createWorldSnapshotPacket(const SnapshotHeader& snapHeader, const std::vector<EntityState>& entities) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT));
        auto payload = snapHeader.serialize();
        for (const auto& entity : entities) {
            auto entityBuf = entity.serialize();
            payload.insert(payload.end(), entityBuf.begin(), entityBuf.end());
        }
        packet.setPayload(payload);
        return packet;
    }

    static std::pair<SnapshotHeader, std::vector<EntityState>> getWorldSnapshot(const NetworkPacket& packet) {
        if (packet.header.type != static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT) || packet.payload.size() < sizeof(SnapshotHeader)) {
            throw std::runtime_error("Invalid payload for WORLD_SNAPSHOT");
        }
        SnapshotHeader snapHeader = SnapshotHeader::deserialize(packet.payload.data());
        std::vector<EntityState> entities;
        size_t offset = sizeof(SnapshotHeader);
        size_t entitySize = sizeof(EntityState);
        
        // Simple validation
         if (packet.payload.size() != offset + snapHeader.entityCount * entitySize) {
            throw std::runtime_error("Payload size mismatch for entities");
        }
        
        for (uint32_t i = 0; i < snapHeader.entityCount; ++i) {
            entities.push_back(EntityState::deserialize(packet.payload.data() + offset + i * entitySize));
        }
        return {snapHeader, entities};
    }
};
