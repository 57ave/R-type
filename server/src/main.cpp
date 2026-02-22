#include <iostream>
#include <thread>
#include <chrono>
#include "network/NetworkServer.hpp"
#include "network/RTypeProtocol.hpp"
#include "ServerConfig.hpp"

int main()
{
    std::cout << "R-Type Server Starting..." << std::endl;

    // Load port from Lua config (single source of truth: server_config.lua)
    ServerConfig::Config cfg;
    ServerConfig::loadFromLua(cfg, "assets/scripts/config/server_config.lua");
    int port = cfg.server.port;
    std::cout << "[Server] Using port " << port << " (from server_config.lua)" << std::endl;

    try {
        NetworkServer server(static_cast<short>(port));
        server.start();

        std::cout << "Server started. Processing packets..." << std::endl;

        while (true) {
            server.process();

            while (server.hasReceivedPackets()) {
                auto [packet, sender] = server.getNextReceivedPacket();
                if (packet.header.type == static_cast<uint16_t>(GamePacketType::CLIENT_INPUT)) {
                    try {
                        ClientInput input = RTypeProtocol::getClientInput(packet);
                        std::cout << "Received input from " << sender << ": playerId=" << (int)input.playerId << ", mask=" << (int)input.inputMask << std::endl;                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing CLIENT_INPUT: " << e.what() << std::endl;
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    } catch (const std::exception& e) {
        std::cerr << "Server Exception: " << e.what() << std::endl;
    }

    return 0;
}
