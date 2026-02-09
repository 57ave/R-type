#include "../../include/network/UdpServer.hpp"
#include <iostream>
#include <sstream>

UdpServer::UdpServer(asio::io_context& io_context, short port)
    : socket_(io_context, udp::endpoint(udp::v4(), port)) {
}

UdpServer::~UdpServer() {
}

void UdpServer::start() {
    startReceive();
    std::cout << "[Server] Listening on port " << socket_.local_endpoint().port() << std::endl;
}

void UdpServer::startReceive() {
    socket_.async_receive_from(
        asio::buffer(recvBuffer_), receiverEndpoint_,
        [this](const std::error_code& error, std::size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        });
}

void UdpServer::handleReceive(const std::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        try {
            NetworkPacket packet = NetworkPacket::deserialize(recvBuffer_.data(), bytes_transferred);

            if (packet.header.magic != 0x5254 || packet.header.version != 1) {
                // Invalid packet, ignore
            } else {
                handleClientSession(receiverEndpoint_, packet);

                std::lock_guard<std::mutex> lock(queueMutex_);
                packetQueue_.push({packet, receiverEndpoint_});
            }

        } catch (const std::exception& e) {
            std::cerr << "[Server] Error parsing packet: " << e.what() << std::endl;
        }
    } else {
        std::cerr << "[Server] Receive error: " << error.message() << std::endl;
    }

    // Continue listening
    startReceive();
}

void UdpServer::handleClientSession(const udp::endpoint& sender, const NetworkPacket& packet) {
    std::stringstream ss;
    ss << sender;
    std::string key = ss.str();

    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = sessions_.find(key);

    if (it == sessions_.end()) {
        // New Client
        // Only accept if it's a CLIENT_HELLO (strict mode) or just accept implicitely (loose mode)
        // For robustness, usually we wait for HELLO, but for now we auto-add.
        
        std::cout << "[Server] New session: " << key << " (ID: " << (int)nextPlayerId_ << ")" << std::endl;
        auto session = std::make_shared<ClientSession>(sender, nextPlayerId_++);
        sessions_[key] = session;
        
        // Auto-reply Welcome could go here or in main loop
    } else {
        // Existing Client: Update Keep Alive
        it->second->updateLastPacketTime();
        
        // Simple Seq check (can be advanced to drop duplicates)
        if (packet.header.seq > it->second->lastSequenceNumber) {
            it->second->lastSequenceNumber = packet.header.seq;
        }
    }
}

bool UdpServer::popPacket(NetworkPacket& outPacket, udp::endpoint& outSender) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (packetQueue_.empty()) {
        return false;
    }
    auto front = packetQueue_.front();
    packetQueue_.pop();
    outPacket = front.first;
    outSender = front.second;
    return true;
}

void UdpServer::sendTo(const NetworkPacket& packet, const udp::endpoint& endpoint) {
    auto buffer = packet.serialize();
    socket_.async_send_to(asio::buffer(buffer), endpoint,
        [](const std::error_code& /*error*/, std::size_t /*bytes_transferred*/) {

        });
}

void UdpServer::broadcast(const NetworkPacket& packet) {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto buffer = packet.serialize();
    
    for (const auto& pair : sessions_) {
        if (pair.second->isConnected) {
            socket_.async_send_to(asio::buffer(buffer), pair.second->endpoint,
                [](const std::error_code&, std::size_t) {});
        }
    }
}

void UdpServer::checkTimeouts() {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    
    // timeout threshold (e.g., 5 seconds)
    auto timeout = std::chrono::seconds(5);

    for (auto it = sessions_.begin(); it != sessions_.end();) {
        if (it->second->isTimedOut(timeout)) {
            std::cout << "[Server] Client timed out: " << it->first << std::endl;
            // Notify others could happen here (CLIENT_LEFT)
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

std::shared_ptr<ClientSession> UdpServer::getSession(const udp::endpoint& endpoint) {
    std::stringstream ss;
    ss << endpoint;
    std::string key = ss.str();

    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = sessions_.find(key);
    if (it != sessions_.end()) {
        return it->second;
    }
    return nullptr;
}

bool UdpServer::removeSession(const udp::endpoint& endpoint) {
    std::stringstream ss;
    ss << endpoint;
    std::string key = ss.str();

    std::lock_guard<std::mutex> lock(sessionsMutex_);
    return sessions_.erase(key) > 0;
}

std::vector<ClientSession> UdpServer::getActiveSessions() const {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    std::vector<ClientSession> result;
    result.reserve(sessions_.size());
    
    for (const auto& pair : sessions_) {
        if (pair.second && pair.second->isConnected) {
            result.push_back(*pair.second);
        }
    }
    
    return result;
}
