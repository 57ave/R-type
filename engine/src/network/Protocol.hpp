#include <cstdint>
#include <stdexcept>
#include <vector>
#include <array>
#include <cstring>
#include <endian.h>

// Little-endian packing for network protocol structures
#pragma pack(push, 1)

// Enum for PacketType (combining client-to-server and server-to-client)
enum class PacketType : uint8_t {
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

// Common PacketHeader struct
struct PacketHeader {
    uint16_t magic;      // 0x5254 ('RT')
    uint8_t  version;    // Protocol version
    PacketType type;     // Packet type
    uint32_t seq;        // Sequence number
    uint32_t timestamp;  // Timestamp in ms

    PacketHeader() : magic(0x5254), version(1), type(PacketType::CLIENT_HELLO), seq(0), timestamp(0) {}

    // Serialize to buffer
    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    // Deserialize from buffer (assumes buffer is at least sizeof(PacketHeader))
    static PacketHeader deserialize(const char* data) {
        PacketHeader header;
        std::memcpy(&header, data, sizeof(header));
        return header;
    }
};

// ClientInput payload
struct ClientInput {
    uint8_t playerId;
    uint8_t inputMask;  // Bitmask for inputs

    ClientInput() : playerId(0), inputMask(0) {}

    // Serialize to buffer
    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    // Deserialize from buffer
    static ClientInput deserialize(const char* data) {
        ClientInput input;
        std::memcpy(&input, data, sizeof(input));
        return input;
    }
};

// SnapshotHeader for WORLD_SNAPSHOT
struct SnapshotHeader {
    uint32_t entityCount;

    SnapshotHeader() : entityCount(0) {}

    // Serialize to buffer
    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    // Deserialize from buffer
    static SnapshotHeader deserialize(const char* data) {
        SnapshotHeader header;
        std::memcpy(&header, data, sizeof(header));
        return header;
    }
};

// EntityState for entities in snapshot
struct EntityState {
    uint32_t id;
    EntityType type;
    float x;
    float y;
    float vx;
    float vy;
    uint8_t hp;  // 0 = dead

    EntityState() : id(0), type(EntityType::ENTITY_PLAYER), x(0.0f), y(0.0f), vx(0.0f), vy(0.0f), hp(0) {}

    // Serialize to buffer
    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    // Deserialize from buffer
    static EntityState deserialize(const char* data) {
        EntityState state;
        std::memcpy(&state, data, sizeof(state));
        return state;
    }
};

#pragma pack(pop)

// Helper class for full packets, to facilitate ASIO usage
class RTypePacket {
public:
    PacketHeader header;

    // Variable payload (e.g., for WORLD_SNAPSHOT, it's SnapshotHeader + vector<EntityState>)
    std::vector<char> payload;

    RTypePacket(PacketType type) {
        header.type = type;
    }

    // Serialize full packet (header + payload)
    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(PacketHeader) + payload.size());
        std::memcpy(buffer.data(), &header, sizeof(PacketHeader));
        if (!payload.empty()) {
            std::memcpy(buffer.data() + sizeof(PacketHeader), payload.data(), payload.size());
        }
        return buffer;
    }

    // Deserialize full packet from buffer
    static RTypePacket deserialize(const char* data, size_t size) {
        if (size < sizeof(PacketHeader)) {
            throw std::runtime_error("Packet too short");
        }
        RTypePacket packet(PacketType::CLIENT_HELLO); // Dummy init
        packet.header = PacketHeader::deserialize(data);
        size_t payloadSize = size - sizeof(PacketHeader);
        if (payloadSize > 0) {
            packet.payload.resize(payloadSize);
            std::memcpy(packet.payload.data(), data + sizeof(PacketHeader), payloadSize);
        }
        return packet;
    }

    // Specific helpers for common packets

    // Set CLIENT_INPUT payload
    void setClientInput(const ClientInput& input) {
        header.type = PacketType::CLIENT_INPUT;
        payload = input.serialize();
    }

    // Get CLIENT_INPUT payload
    ClientInput getClientInput() const {
        if (header.type != PacketType::CLIENT_INPUT || payload.size() != sizeof(ClientInput)) {
            throw std::runtime_error("Invalid payload for CLIENT_INPUT");
        }
        return ClientInput::deserialize(payload.data());
    }

    // Set WORLD_SNAPSHOT payload (header + entities)
    void setWorldSnapshot(const SnapshotHeader& snapHeader, const std::vector<EntityState>& entities) {
        header.type = PacketType::WORLD_SNAPSHOT;
        payload = snapHeader.serialize();
        for (const auto& entity : entities) {
            auto entityBuf = entity.serialize();
            payload.insert(payload.end(), entityBuf.begin(), entityBuf.end());
        }
    }

    // Get WORLD_SNAPSHOT payload
    std::pair<SnapshotHeader, std::vector<EntityState>> getWorldSnapshot() const {
        if (header.type != PacketType::WORLD_SNAPSHOT || payload.size() < sizeof(SnapshotHeader)) {
            throw std::runtime_error("Invalid payload for WORLD_SNAPSHOT");
        }
        SnapshotHeader snapHeader = SnapshotHeader::deserialize(payload.data());
        std::vector<EntityState> entities;
        size_t offset = sizeof(SnapshotHeader);
        size_t entitySize = sizeof(EntityState);
        if (payload.size() != offset + snapHeader.entityCount * entitySize) {
            throw std::runtime_error("Payload size mismatch for entities");
        }
        for (uint32_t i = 0; i < snapHeader.entityCount; ++i) {
            entities.push_back(EntityState::deserialize(payload.data() + offset + i * entitySize));
        }
        return {snapHeader, entities};
    }

    // Add similar methods for other packet types as needed (e.g., ENTITY_SPAWN could reuse EntityState)
};

// Example usage with Boost.Asio (not part of the classes, but for illustration):
/*
#include <boost/asio.hpp>

void sendPacket(boost::asio::ip::udp::socket& socket, const boost::asio::ip::udp::endpoint& endpoint, const RTypePacket& packet) {
    auto buffer = packet.serialize();
    socket.send_to(boost::asio::buffer(buffer), endpoint);
}

RTypePacket receivePacket(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint& sender) {
    std::array<char, 65536> recvBuffer; // Max UDP size
    size_t len = socket.receive_from(boost::asio::buffer(recvBuffer), sender);
    return RTypePacket::deserialize(recvBuffer.data(), len);
}
*/