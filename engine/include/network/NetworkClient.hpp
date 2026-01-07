#pragma once

#include <asio.hpp>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include "UdpClient.hpp"
#include "Packet.hpp"
#include "RTypeProtocol.hpp"

class NetworkClient {
public:
    NetworkClient(const std::string& serverAddress, short serverPort);
    ~NetworkClient();

    void start();
    void process();
    void disconnect();

    // Send input to server
    void sendInput(uint8_t playerId, uint8_t inputMask, uint8_t chargeLevel = 0);

    // Send HELLO to server
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
