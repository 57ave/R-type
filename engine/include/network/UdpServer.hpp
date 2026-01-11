#pragma once

#include <asio.hpp>
#include <vector>
#include <map>
#include <mutex>
#include <iostream>
#include <queue>
#include "Packet.hpp"
#include "ClientSession.hpp"
#include "RTypeProtocol.hpp" // For GamePacketType

using asio::ip::udp;

class UdpServer {
public:
    UdpServer(asio::io_context& io_context, short port);
    ~UdpServer();


    void start();


    bool popPacket(NetworkPacket& outPacket, udp::endpoint& outSender);


    void broadcast(const NetworkPacket& packet);


    void sendTo(const NetworkPacket& packet, const udp::endpoint& endpoint);


    void checkTimeouts();
    
    std::shared_ptr<ClientSession> getSession(const udp::endpoint& endpoint);

private:
    void startReceive();
    void handleReceive(const std::error_code& error, std::size_t bytes_transferred);
    

    void handleClientSession(const udp::endpoint& sender, const NetworkPacket& packet);

private:
    udp::socket socket_;
    udp::endpoint receiverEndpoint_;
    std::array<char, 65536> recvBuffer_;


    std::map<std::string, std::shared_ptr<ClientSession>> sessions_; // Key: "IP:Port"
    std::mutex sessionsMutex_;
    uint8_t nextPlayerId_ = 1;


    std::queue<std::pair<NetworkPacket, udp::endpoint>> packetQueue_;
    std::mutex queueMutex_;
};