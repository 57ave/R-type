#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include "Serializer.hpp"

#ifdef __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#pragma pack(push, 1)

struct PacketHeader {
    uint16_t magic;      // 0x5254 ('RT')
    uint8_t  version;    // Protocol version
    uint8_t  flags;      // Flags (1 = Compressed)
    uint16_t type;       // Packet type
    uint32_t seq;        // Sequence number
    uint32_t timestamp;  // Timestamp in ms

    PacketHeader() : magic(0x5254), version(1), flags(0), type(0), seq(0), timestamp(0) {}



    // Serialize to buffer
    std::vector<char> serialize() const {
        Network::Serializer serializer;
        serializer.write(*this);
        return serializer.getBuffer();
    }

    // Deserialize from buffer
    static PacketHeader deserialize(const char* data) {
        Network::Deserializer deserializer(data, sizeof(PacketHeader));
        return deserializer.read<PacketHeader>();
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
        Network::Serializer serializer;
        serializer.write(header);
        if (!payload.empty()) {
            serializer.writeBytes(payload.data(), payload.size());
        }
        return serializer.getBuffer();
    }

    // Deserialize full packet from buffer
    static NetworkPacket deserialize(const char* data, size_t size) {
        if (size < sizeof(PacketHeader)) {
            throw std::runtime_error("Packet too short");
        }
        Network::Deserializer deserializer(data, size);
        
        NetworkPacket packet;
        packet.header = deserializer.read<PacketHeader>();
        
        // Remaining bytes are payload
        size_t payloadSize = size - sizeof(PacketHeader);
        if (payloadSize > 0) {
            packet.payload = deserializer.readBytes(payloadSize);
        }
        return packet;
    }
    
    void setPayload(const std::vector<char>& newPayload) {
        payload = newPayload;
    }
};
