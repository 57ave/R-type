#pragma once

#include <asio.hpp>
#include <iostream>
#include <mutex>
#include <queue>
#include <vector>

#include "Packet.hpp"

using asio::ip::udp;

class UdpClient {
public:
    UdpClient(asio::io_context& io_context, const std::string& serverAddress, short serverPort);
    ~UdpClient();

    // Start receiving packets
    void start();

    // Send a packet to the server
    void send(const NetworkPacket& packet);

    // Pop the next received packet from the queue (Thread-safe)
    // Returns true if a packet was retrieved, false if queue is empty
    bool popPacket(NetworkPacket& outPacket);

    // Check if connected
    bool isConnected() const { return connected_; }

private:
    void startReceive();
    void handleReceive(const std::error_code& error, std::size_t bytes_transferred);
    void handleSend(const std::error_code& error, std::size_t bytes_transferred);

private:
    udp::socket socket_;
    udp::endpoint serverEndpoint_;
    std::array<char, 65536> recvBuffer_;
    bool connected_;

    // Packet queue for Game Engine
    std::queue<NetworkPacket> packetQueue_;
    std::mutex queueMutex_;
};
