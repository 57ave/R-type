#include <iostream>
#include <boost/asio.hpp>
#include "Protocol.hpp"  // Assume the previous classes are in this header

using boost::asio::ip::udp;

// Helper functions from previous example
void sendPacket(udp::socket& socket, const udp::endpoint& endpoint, const RTypePacket& packet) {
    auto buffer = packet.serialize();
    socket.send_to(boost::asio::buffer(buffer), endpoint);
}

RTypePacket receivePacket(udp::socket& socket, udp::endpoint& sender) {
    std::array<char, 65536> recvBuffer;  // Max UDP size
    size_t len = socket.receive_from(boost::asio::buffer(recvBuffer), sender);
    return RTypePacket::deserialize(recvBuffer.data(), len);
}

// Server example
void runServer() {
    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345));  // Bind to port 12345

    std::cout << "Server listening on port 12345..." << std::endl;

    while (true) {
        udp::endpoint client_endpoint;
        RTypePacket packet = receivePacket(socket, client_endpoint);

        if (packet.header.magic != 0x5254 || packet.header.version != 1) {
            std::cout << "Invalid packet received." << std::endl;
            continue;
        }

        if (packet.header.type == PacketType::CLIENT_HELLO) {
            std::cout << "Received CLIENT_HELLO from " << client_endpoint << std::endl;

            // Prepare SERVER_WELCOME (no payload assumed for this example)
            RTypePacket welcome(PacketType::SERVER_WELCOME);
            welcome.header.seq = packet.header.seq;  // Echo seq or increment as needed
            welcome.header.timestamp = static_cast<uint32_t>(time(nullptr) * 1000);  // Example timestamp

            sendPacket(socket, client_endpoint, welcome);
            std::cout << "Sent SERVER_WELCOME to " << client_endpoint << std::endl;
        }
    }
}

// Client example
void runClient() {
    boost::asio::io_context io_context;
    udp::socket socket(io_context);
    socket.open(udp::v4());

    udp::resolver resolver(io_context);
    udp::endpoint server_endpoint = *resolver.resolve("localhost", "12345").begin();

    // Prepare CLIENT_HELLO (no payload)
    RTypePacket hello(PacketType::CLIENT_HELLO);
    hello.header.seq = 1;  // Starting sequence
    hello.header.timestamp = static_cast<uint32_t>(time(nullptr) * 1000);

    sendPacket(socket, server_endpoint, hello);
    std::cout << "Sent CLIENT_HELLO to server." << std::endl;

    udp::endpoint sender_endpoint;
    RTypePacket response = receivePacket(socket, sender_endpoint);

    if (response.header.type == PacketType::SERVER_WELCOME) {
        std::cout << "Received SERVER_WELCOME from server." << std::endl;
    } else {
        std::cout << "Unexpected response." << std::endl;
    }
}

// Main to run either server or client (for demo, run server in one terminal, client in another)
int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "server") {
        runServer();
    } else {
        runClient();
    }
    return 0;
}