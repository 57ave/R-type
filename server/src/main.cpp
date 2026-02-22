#include "core/Logger.hpp"
#include <thread>
#include <chrono>
#include "network/NetworkServer.hpp"
#include "network/RTypeProtocol.hpp"
#include "ServerConfig.hpp"

int main()
{
    LOG_INFO("MAIN", "R-Type Server Starting...");

    // Load port from Lua config (single source of truth: server_config.lua)
    ServerConfig::Config cfg;
    ServerConfig::loadFromLua(cfg, "assets/scripts/config/server_config.lua");
    int port = cfg.server.port;
    LOG_INFO("SERVER", "Using port " + std::to_string(port) + " (from server_config.lua)");

    try {
        NetworkServer server(static_cast<short>(port));
        server.start();

        LOG_INFO("MAIN", "Server started. Processing packets...");

        while (true) {
            server.process();

            while (server.hasReceivedPackets()) {
                auto [packet, sender] = server.getNextReceivedPacket();
                if (packet.header.type == static_cast<uint16_t>(GamePacketType::CLIENT_INPUT)) {
                    try {
                        ClientInput input = RTypeProtocol::getClientInput(packet);
                        LOG_INFO("MAIN", "Received input from " + std::to_string(sender) + ": playerId=" + std::to_string((int)input.playerId) + ", mask=" + std::to_string((int)input.inputMask) + std::to_string(std::endl;                    } catch (const std::exception& e) {));
                        LOG_ERROR("MAIN", "Error parsing CLIENT_INPUT: " + std::string(e.what()));
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    } catch (const std::exception& e) {
        LOG_ERROR("MAIN", "Server Exception: " + std::string(e.what()));
    }

    return 0;
}
