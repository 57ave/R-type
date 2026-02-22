#include "network/NetworkClient.hpp"
#include "core/Logger.hpp"

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
    LOG_INFO("NETWORKCLIENT", "Destructor called!");
    disconnect();
}

void NetworkClient::start() {
    client_.start();
    
    // Start io_context in a separate thread
    LOG_INFO("NETWORKCLIENT", "Starting io_context thread...");
    io_thread_ = std::thread([this]() {
        LOG_INFO("NETWORKCLIENT", "io_context thread started, running io_context...");
        try {
            io_context_.run();
            LOG_INFO("NETWORKCLIENT", "io_context.run() exited");
        } catch (const std::exception& e) {
            LOG_ERROR("NETWORKCLIENT", std::string("io_context exception: ") + e.what());
        }
    });
    
    connected_ = true;
    LOG_INFO("NETWORKCLIENT", "Started");
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
    LOG_INFO("NETWORKCLIENT", std::string("disconnect() called, connected_=") + (connected_ ? "true" : "false"));
    if (connected_) {
        // Send disconnect packet (type 0x04 is common convention)
        NetworkPacket packet(0x04);  // CLIENT_DISCONNECT
        packet.header.seq = sequenceNumber_++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
        
        client_.send(packet);
        connected_ = false;

        // Give the async send a moment to flush the disconnect packet
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
        LOG_INFO("NETWORKCLIENT", "Disconnected");
    }

    // Close the UDP socket to cancel all pending async operations
    client_.close();

    // Stop the io_context so io_context_.run() returns
    io_context_.stop();

    // Wait for the io_context thread to finish
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}

// Generic packet send - game should create NetworkPacket with their specific protocol
void NetworkClient::sendPacket(const NetworkPacket& packet) {
    if (!connected_) return;
    client_.send(packet);
    lastInputSent_ = std::chrono::steady_clock::now();
}

void NetworkClient::sendHello() {
    // Generic HELLO packet - type 0x01 is common convention
    NetworkPacket packet(0x01);  // CLIENT_HELLO
    packet.header.seq = sequenceNumber_++;
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    client_.send(packet);
    LOG_INFO("NETWORKCLIENT", "Sent CLIENT_HELLO");
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
    if (!connected_) return;

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastPing = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPingSent_).count();

    // Send ping every 2 seconds to prevent server timeout (server has 5s timeout)
    if (timeSinceLastPing > 2000) {
        // Send a ping/keep-alive packet (type 0x03 is CLIENT_PING)
        NetworkPacket pingPacket(0x03);
        pingPacket.header.seq = sequenceNumber_++;
        pingPacket.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
        
        client_.send(pingPacket);
        lastPingSent_ = now;
        LOG_INFO("NETWORKCLIENT", "Sent CLIENT_PING (keep-alive)");
    }
}
