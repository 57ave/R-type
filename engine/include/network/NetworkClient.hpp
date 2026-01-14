#pragma once

#include <asio.hpp>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include "UdpClient.hpp"
#include "Packet.hpp"
// Removed: "GameProtocol.hpp" - Engine should not depend on game-specific protocol
// Game-specific methods (sendInput, etc.) should be in a game wrapper class

class NetworkClient {
public:
    NetworkClient(const std::string& serverAddress, short serverPort);
    ~NetworkClient();

    void start();
    void process();
    void disconnect();

    // Generic packet send - game wraps this with their protocol
    void sendPacket(const NetworkPacket& packet);

    // Send HELLO to server (can be made generic with packet type parameter)
    void sendHello();

    // Check if packets are available
    bool hasReceivedPackets();
    NetworkPacket getNextReceivedPacket();

    // Connection status
    bool isConnected() const { return connected_; }
    uint8_t getPlayerId() const { return playerId_; }
    void setPlayerId(uint8_t id) { playerId_ = id; }

private:
    asio::io_context io_context_;
    std::thread io_thread_;
    UdpClient client_;
    
    std::queue<NetworkPacket> receivedPackets_;
    std::mutex packetsMutex_;
    
    uint32_t sequenceNumber_;
    uint8_t playerId_;
    bool connected_;
    
    std::chrono::steady_clock::time_point lastInputSent_;
    std::chrono::steady_clock::time_point lastPingSent_;
};
