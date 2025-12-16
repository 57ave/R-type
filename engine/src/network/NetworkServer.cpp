#include <iostream>
#include <thread>
#include <chrono>
#include "UdpServer.hpp"

void runServer() {
    try {
        asio::io_context io_context;
        
        UdpServer server(io_context, 12345);
        server.start();

        std::thread ioThread([&io_context](){
            io_context.run();
        });

        std::cout << "Server started. Loop running..." << std::endl;

        while (true) {
            NetworkPacket packet;
            udp::endpoint sender;
            
            while (server.popPacket(packet, sender)) {
                if (packet.header.type == static_cast<uint16_t>(GamePacketType::CLIENT_HELLO)) {
                    // Send Welcome
                    NetworkPacket welcome(static_cast<uint16_t>(GamePacketType::SERVER_WELCOME));
                    welcome.header.seq = packet.header.seq;
                    server.sendTo(welcome, sender);
                    std::cout << "Sent Welcome to " << sender << std::endl;
                }
                else if (packet.header.type == static_cast<uint16_t>(GamePacketType::CLIENT_INPUT)) {
                    // Handle Input
                     try {
                        ClientInput input = RTypeProtocol::getClientInput(packet);
                        // std::cout << "Input received: " << (int)input.inputMask << std::endl;
                        // TODO: Apply to ECS
                    } catch (...) {}
                }
            }

            server.checkTimeouts();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        ioThread.join();

    } catch (const std::exception& e) {
        std::cerr << "Server Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    runServer();
    return 0;
}