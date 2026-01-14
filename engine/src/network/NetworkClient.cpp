#include "network/NetworkClient.hpp"
#include <iostream>

NetworkClient::NetworkClient(const std::string& serverAddress, short serverPort)
    : client_(io_context_, serverAddress, serverPort),
      sequenceNumber_(0),
      playerId_(0),
      connected_(false),
      lastInputSent_(std::chrono::steady_clock::now()),
      lastPingSent_(std::chrono::steady_clock::now())
{
}

NetworkClient::~NetworkClient() {
    disconnect();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}

void NetworkClient::start() {
    client_.start();
    
    // Start io_context in a separate thread
    io_thread_ = std::thread([this]() {
        io_context_.run();
    });
    
    connected_ = true;
    std::cout << "[NetworkClient] Started" << std::endl;
}

void NetworkClient::process() {
    // Process received packets from UdpClient
    NetworkPacket packet;
    while (client_.popPacket(packet)) {
        std::lock_guard<std::mutex> lock(packetsMutex_);
        receivedPackets_.push(packet);
    }
}

void NetworkClient::disconnect() {
    if (connected_) {
        // Send disconnect packet
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CLIENT_DISCONNECT));
        packet.header.seq = sequenceNumber_++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
        
        client_.send(packet);
        connected_ = false;
        
        std::cout << "[NetworkClient] Disconnected" << std::endl;
    }
}

void NetworkClient::sendInput(uint8_t playerId, uint8_t inputMask) {
    if (!connected_) return;

    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CLIENT_INPUT));
    packet.header.seq = sequenceNumber_++;
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    ClientInput input;
    input.playerId = playerId;
    input.inputMask = inputMask;
    packet.setPayload(input.serialize());

    client_.send(packet);
    lastInputSent_ = std::chrono::steady_clock::now();
}

void NetworkClient::sendHello() {
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CLIENT_HELLO));
    packet.header.seq = sequenceNumber_++;
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    client_.send(packet);
    std::cout << "[NetworkClient] Sent CLIENT_HELLO" << std::endl;
}

void NetworkClient::sendPacket(const NetworkPacket& packet) {
    if (!connected_) return;
    
    NetworkPacket p = packet;
    p.header.seq = sequenceNumber_++;
    p.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    client_.send(p);
}

bool NetworkClient::hasReceivedPackets() {
    std::lock_guard<std::mutex> lock(packetsMutex_);
    return !receivedPackets_.empty();
}

NetworkPacket NetworkClient::getNextReceivedPacket() {
    std::lock_guard<std::mutex> lock(packetsMutex_);
    if (receivedPackets_.empty()) {
        throw std::runtime_error("No packets available");
    }
    NetworkPacket packet = receivedPackets_.front();
    receivedPackets_.pop();
    return packet;
}

void NetworkClient::update(float dt) {
    if (!connected_) return;

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastInput = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputSent_).count();

    if (timeSinceLastInput > 1000) {
        sendInput(playerId_, 0);
        sendInput(playerId_, 0);
    }
}
