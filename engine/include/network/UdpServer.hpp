#pragma once

#include <asio.hpp>
#include <vector>
#include <map>
#include <mutex>
#include <iostream>
#include <queue>
#include "Packet.hpp"
#include "ClientSession.hpp"
// Removed: "GameProtocol.hpp" - Engine should not depend on game-specific protocol

using asio::ip::udp;

class UdpServer {
public:
    UdpServer(asio::io_context& io_context, short port);
    ~UdpServer();

    // Start receiving packets
    void start();

    // Pop the next valid packet from the queue (Thread-safe)
    // Returns true if a packet was retrieved, false if queue is empty
    bool popPacket(NetworkPacket& outPacket, udp::endpoint& outSender);

    // Broadcast a packet to all connected clients
    void broadcast(const NetworkPacket& packet);

    // Send to specific client
    void sendTo(const NetworkPacket& packet, const udp::endpoint& endpoint);

    // Check for timeouts and remove inactive clients
    void checkTimeouts();
    
    std::shared_ptr<ClientSession> getSession(const udp::endpoint& endpoint);
    bool removeSession(const udp::endpoint& endpoint);
    std::vector<ClientSession> getActiveSessions() const;

private:
    void startReceive();
    void handleReceive(const std::error_code& error, std::size_t bytes_transferred);
    
    // Internal helper to get or create session
    void handleClientSession(const udp::endpoint& sender, const NetworkPacket& packet);

private:
    udp::socket socket_;
    udp::endpoint receiverEndpoint_;
    std::array<char, 65536> recvBuffer_;

    // Client management
    std::map<std::string, std::shared_ptr<ClientSession>> sessions_; // Key: "IP:Port"
    mutable std::mutex sessionsMutex_;  // mutable to allow const methods to lock
    uint8_t nextPlayerId_ = 1;

    // Packet queue for Game Engine
    std::queue<std::pair<NetworkPacket, udp::endpoint>> packetQueue_;
    mutable std::mutex queueMutex_;  // mutable to allow const methods to lock
};