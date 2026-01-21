#include "network/NetworkClient.hpp"

#include <iostream>

NetworkClient::NetworkClient(const std::string& serverAddress, short serverPort)
    : client_(io_context_, serverAddress, serverPort), sequenceNumber_(0), playerId_(0),
      connected_(false), lastInputSent_(std::chrono::steady_clock::now()),
      lastPingSent_(std::chrono::steady_clock::now()) {}

NetworkClient::~NetworkClient() {
    disconnect();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}

void NetworkClient::start() {
    client_.start();

    // Start io_context in a separate thread
    io_thread_ = std::thread([this]() { io_context_.run(); });

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
        try {
            // Send disconnect packet (type 0x04 is common convention)
            NetworkPacket packet(0x04);  // CLIENT_DISCONNECT
            packet.header.seq = sequenceNumber_++;
            packet.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                          std::chrono::steady_clock::now().time_since_epoch())
                                          .count();

            client_.send(packet);
        } catch (const std::exception& e) {
            // Ignore errors during disconnect (socket may already be closed)
            std::cerr << "[NetworkClient] Error during disconnect (ignored): " << e.what() << std::endl;
        }
        
        connected_ = false;

        std::cout << "[NetworkClient] Disconnected" << std::endl;
    }
}

// Generic packet send - game should create NetworkPacket with their specific protocol
void NetworkClient::sendPacket(const NetworkPacket& packet) {
    if (!connected_)
        return;
    client_.send(packet);
    lastInputSent_ = std::chrono::steady_clock::now();
}

void NetworkClient::sendHello() {
    // Generic HELLO packet - type 0x01 is common convention
    NetworkPacket packet(0x01);  // CLIENT_HELLO
    packet.header.seq = sequenceNumber_++;
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::steady_clock::now().time_since_epoch())
                                  .count();

    client_.send(packet);
    std::cout << "[NetworkClient] Sent CLIENT_HELLO" << std::endl;
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

void NetworkClient::update(float) {
    if (!connected_)
        return;

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastInput =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputSent_).count();

    // Keep-alive: send empty packet if no input for 1 second
    if (timeSinceLastInput > 1000) {
        // Send a ping/keep-alive packet (type 0x03 is CLIENT_PING)
        NetworkPacket pingPacket(0x03);
        sendPacket(pingPacket);
    }
}
