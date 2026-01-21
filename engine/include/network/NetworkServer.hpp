#pragma once

#include <asio.hpp>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

#include "Packet.hpp"
#include "RoomManager.hpp"
#include "UdpServer.hpp"

class NetworkServer {
public:
    NetworkServer(short port);
    ~NetworkServer();

    void start();
    void process();

    bool hasReceivedPackets();
    std::pair<NetworkPacket, asio::ip::udp::endpoint> getNextReceivedPacket();

    RoomManager& getRoomManager() { return roomManager_; }

    void broadcast(const NetworkPacket& packet);
    void sendTo(const NetworkPacket& packet, const asio::ip::udp::endpoint& endpoint);
    void checkTimeouts();

    void removeClient(const asio::ip::udp::endpoint& endpoint);
    std::shared_ptr<ClientSession> getSession(const asio::ip::udp::endpoint& endpoint);
    std::vector<ClientSession> getActiveSessions() const;

private:
    asio::io_context io_context_;
    std::thread io_thread_;
    UdpServer server_;
    std::queue<std::pair<NetworkPacket, asio::ip::udp::endpoint>> receivedPackets_;
    std::mutex packetsMutex_;
    RoomManager roomManager_;
};