#pragma once

#include <asio.hpp>
#include <chrono>

using asio::ip::udp;

class ClientSession {
public:
    udp::endpoint endpoint;
    std::chrono::steady_clock::time_point lastPacketTime;
    uint32_t lastSequenceNumber;
    uint8_t playerId;
    bool isConnected;

    ClientSession(udp::endpoint ep, uint8_t id) 
        : endpoint(ep), 
          lastPacketTime(std::chrono::steady_clock::now()), 
          lastSequenceNumber(0), 
          playerId(id), 
          isConnected(true) {}

    void updateLastPacketTime() {
        lastPacketTime = std::chrono::steady_clock::now();
    }

    bool isTimedOut(const std::chrono::milliseconds& timeoutDuration) const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPacketTime) > timeoutDuration;
    }
};