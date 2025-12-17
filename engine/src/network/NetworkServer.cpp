#include "network/NetworkServer.hpp"
#include <iostream>

NetworkServer::NetworkServer(short port)
    : server_(io_context_, port) {
}

NetworkServer::~NetworkServer() {
    io_context_.stop();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}

void NetworkServer::start() {
    server_.start();
    io_thread_ = std::thread([this]() {
        io_context_.run();
    });
}

void NetworkServer::process() {
    NetworkPacket packet;
    udp::endpoint sender;

    while (server_.popPacket(packet, sender)) {
        // Buffer all packets for polling by the game server
        std::lock_guard<std::mutex> lock(packetsMutex_);
        receivedPackets_.push({packet, sender});
    }

    server_.checkTimeouts();
}

bool NetworkServer::hasReceivedPackets() {
    std::lock_guard<std::mutex> lock(packetsMutex_);
    return !receivedPackets_.empty();
}

std::pair<NetworkPacket, asio::ip::udp::endpoint> NetworkServer::getNextReceivedPacket() {
    std::lock_guard<std::mutex> lock(packetsMutex_);
    if (receivedPackets_.empty()) {
        throw std::runtime_error("No packets available");
    }
    auto packet = receivedPackets_.front();
    receivedPackets_.pop();
    return packet;
}

void NetworkServer::broadcast(const NetworkPacket& packet) {
    server_.broadcast(packet);
}

void NetworkServer::sendTo(const NetworkPacket& packet, const asio::ip::udp::endpoint& endpoint) {
    server_.sendTo(packet, endpoint);
}

void NetworkServer::checkTimeouts() {
    server_.checkTimeouts();
}