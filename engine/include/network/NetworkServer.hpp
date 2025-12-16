#pragma once

#include <asio.hpp>
#include <queue>
#include <mutex>
#include <thread>
#include <utility>
#include "UdpServer.hpp"
#include "Packet.hpp"

class NetworkServer {
public:
    NetworkServer(short port);
    ~NetworkServer();

    void start();
    void process();

    bool hasReceivedPackets();
    std::pair<NetworkPacket, asio::ip::udp::endpoint> getNextReceivedPacket();

private:
    asio::io_context io_context_;
    std::thread io_thread_;
    UdpServer server_;
    std::queue<std::pair<NetworkPacket, asio::ip::udp::endpoint>> receivedPackets_;
    std::mutex packetsMutex_;
};