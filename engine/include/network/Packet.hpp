#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>

#ifdef __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#pragma pack(push, 1)

struct PacketHeader {
    uint16_t magic;      // 0x5254 ('RT') but could be anything generic
    uint8_t  version;    // Protocol version
    uint16_t type;       // Generic Packet type (ID) - User casts this to their specific Enum
    uint32_t seq;        // Sequence number
    uint32_t timestamp;  // Timestamp in ms

    PacketHeader() : magic(0x5254), version(1), type(0), seq(0), timestamp(0) {}

    // Serialize to buffer
    std::vector<char> serialize() const {
        std::vector<char> buffer(sizeof(*this));
        std::memcpy(buffer.data(), this, sizeof(*this));
        return buffer;
    }

    // Deserialize from buffer
    static PacketHeader deserialize(const char* data) {
        PacketHeader header;
        std::memcpy(&header, data, sizeof(header));
        return header;
    }
};

#pragma pack(pop)

// Generic Packet class (Header + Payload)
class NetworkPacket {
public:
    PacketHeader header;
    std::vector<char> payload;

    NetworkPacket(uint16_t type = 0) {
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
    static NetworkPacket deserialize(const char* data, size_t size) {
        if (size < sizeof(PacketHeader)) {
            throw std::runtime_error("Packet too short");
        }
        NetworkPacket packet;
        packet.header = PacketHeader::deserialize(data);
        size_t payloadSize = size - sizeof(PacketHeader);
        if (payloadSize > 0) {
            packet.payload.resize(payloadSize);
            std::memcpy(packet.payload.data(), data + sizeof(PacketHeader), payloadSize);
        }
        return packet;
    }
    
    void setPayload(const std::vector<char>& newPayload) {
        payload = newPayload;
    }
};
