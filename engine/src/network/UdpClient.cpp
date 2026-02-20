#include "network/UdpClient.hpp"

UdpClient::UdpClient(asio::io_context& io_context, const std::string& serverAddress, short serverPort)
    : socket_(io_context, udp::endpoint(udp::v4(), 0)), // Bind to any port
      connected_(false)
{
    // Resolve server address
    udp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(udp::v4(), serverAddress, std::to_string(serverPort));
    serverEndpoint_ = *endpoints.begin();
    
    connected_ = true;
    std::cout << "[UdpClient] Initialized. Server: " << serverEndpoint_ << std::endl;
}

UdpClient::~UdpClient() {
    close();
}

void UdpClient::close() {
    connected_ = false;
    if (socket_.is_open()) {
        std::error_code ec;
        socket_.cancel(ec);  // Cancel pending async operations
        socket_.close(ec);
    }
}

void UdpClient::start() {
    startReceive();
}

void UdpClient::send(const NetworkPacket& packet) {
    if (!socket_.is_open()) return;
    try {
        auto buffer = packet.serialize();
        std::cout << "[UdpClient] Sending packet type " << packet.header.type 
                  << " (" << buffer.size() << " bytes) to " << serverEndpoint_ << std::endl;
        socket_.async_send_to(
            asio::buffer(buffer),
            serverEndpoint_,
            [this](const std::error_code& error, std::size_t bytes_transferred) {
                handleSend(error, bytes_transferred);
            }
        );
    } catch (const std::exception& e) {
        std::cerr << "[UdpClient] Send error: " << e.what() << std::endl;
    }
}

bool UdpClient::popPacket(NetworkPacket& outPacket) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (packetQueue_.empty()) {
        return false;
    }
    outPacket = packetQueue_.front();
    packetQueue_.pop();
    return true;
}

void UdpClient::startReceive() {
    if (!socket_.is_open()) return;
    socket_.async_receive_from(
        asio::buffer(recvBuffer_),
        serverEndpoint_,
        [this](const std::error_code& error, std::size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        }
    );
}

void UdpClient::handleReceive(const std::error_code& error, std::size_t bytes_transferred) {
    if (error) {
        // Socket was closed or operation cancelled - stop the receive loop
        if (error == asio::error::operation_aborted || !socket_.is_open()) {
            return;
        }
        std::cerr << "[UdpClient] Receive error: " << error.message() << std::endl;
        startReceive();
        return;
    }

    if (bytes_transferred > 0) {
        try {
            NetworkPacket packet = NetworkPacket::deserialize(recvBuffer_.data(), bytes_transferred);
            
            // Validate magic number
            if (packet.header.magic != 0x5254) {
                std::cerr << "[UdpClient] Invalid magic number" << std::endl;
                startReceive();
                return;
            }

            // Add to queue
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                packetQueue_.push(packet);
            }

        } catch (const std::exception& e) {
            std::cerr << "[UdpClient] Receive parse error: " << e.what() << std::endl;
        }
    }

    // Continue receiving
    startReceive();
}

void UdpClient::handleSend(const std::error_code& error, std::size_t bytes_transferred) {
    if (error) {
        std::cerr << "[UdpClient] Send error: " << error.message() << std::endl;
    } else {
        std::cout << "[UdpClient] Successfully sent " << bytes_transferred << " bytes" << std::endl;
    }
}
