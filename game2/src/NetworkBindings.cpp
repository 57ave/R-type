#include "NetworkBindings.hpp"
#include <scripting/LuaState.hpp>
#include <iostream>

namespace FlappyBird {

// Enum pour les types de paquets du jeu Flappy Bird
// MUST MATCH SimpleProtocol.hpp on server side!
enum class FlappyPacketType : uint16_t {
    // Connection (used by NetworkClient auto-send)
    CLIENT_HELLO    = 0x01,
    SERVER_WELCOME  = 0x02,
    CLIENT_PING     = 0x03,
    
    // Game Control
    GAME_START      = 0x10,
    START_COUNTDOWN = 0x11,
    GAME_OVER       = 0x12,
    
    // Gameplay
    PLAYER_INPUT    = 0x20,
    GAME_STATE      = 0x21,
    SPAWN_PIPE      = 0x22,
    PLAYER_DIED     = 0x23,
};

void NetworkBindings::RegisterPacketTypes(sol::state& lua) {
    // Créer une table pour les types de paquets
    lua["PacketType"] = lua.create_table();
    lua["PacketType"]["CLIENT_HELLO"] = static_cast<int>(FlappyPacketType::CLIENT_HELLO);
    lua["PacketType"]["SERVER_WELCOME"] = static_cast<int>(FlappyPacketType::SERVER_WELCOME);
    lua["PacketType"]["CLIENT_PING"] = static_cast<int>(FlappyPacketType::CLIENT_PING);
    lua["PacketType"]["GAME_START"] = static_cast<int>(FlappyPacketType::GAME_START);
    lua["PacketType"]["START_COUNTDOWN"] = static_cast<int>(FlappyPacketType::START_COUNTDOWN);
    lua["PacketType"]["GAME_OVER"] = static_cast<int>(FlappyPacketType::GAME_OVER);
    lua["PacketType"]["PLAYER_INPUT"] = static_cast<int>(FlappyPacketType::PLAYER_INPUT);
    lua["PacketType"]["GAME_STATE"] = static_cast<int>(FlappyPacketType::GAME_STATE);
    lua["PacketType"]["SPAWN_PIPE"] = static_cast<int>(FlappyPacketType::SPAWN_PIPE);
    lua["PacketType"]["PLAYER_DIED"] = static_cast<int>(FlappyPacketType::PLAYER_DIED);
}

void NetworkBindings::RegisterNetworkClient(sol::state& lua, NetworkClient* client) {
    // Get or create Network table (preserve existing table if it exists)
    sol::table networkTable;
    if (lua["Network"].valid() && lua["Network"].get_type() == sol::type::table) {
        networkTable = lua["Network"];
        std::cout << "[NetworkBindings] Using existing Network table" << std::endl;
    } else {
        networkTable = lua.create_table();
        lua["Network"] = networkTable;
        std::cout << "[NetworkBindings] Created new Network table" << std::endl;
    }
    
    if (!client) {
        std::cerr << "[NetworkBindings] Warning: NetworkClient is null - only stub functions will be available" << std::endl;
        
        // Stub connect function that returns error
        networkTable["connect"] = [](const std::string& ip, int port) -> bool {
            std::cerr << "[Network] ERROR: NetworkClient not available" << std::endl;
            return false;
        };
        
        return;
    }

    // Fonction de connexion
    networkTable["connect"] = [client](const std::string& ip, int port) -> bool {
        try {
            std::cout << "[Network] Connecting to " << ip << ":" << port << std::endl;
            // Note: NetworkClient est déjà instancié avec l'IP/port dans le constructeur
            client->start();
            client->sendHello();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Network] Connection error: " << e.what() << std::endl;
            return false;
        }
    };

    // Fonction de déconnexion
    networkTable["disconnect"] = [client]() {
        std::cout << "[Network] Disconnecting..." << std::endl;
        client->disconnect();
    };

    // Vérifier si connecté
    networkTable["isConnected"] = [client]() -> bool {
        return client->isConnected();
    };

    // Obtenir l'ID du joueur
    networkTable["getPlayerId"] = [client]() -> int {
        return static_cast<int>(client->getPlayerId());
    };

    // Envoyer un input (flap)
    networkTable["sendFlap"] = [client](int playerId) {
        NetworkPacket packet(static_cast<uint16_t>(FlappyPacketType::PLAYER_INPUT));
        packet.payload.push_back(static_cast<char>(playerId));
        client->sendPacket(packet);
        std::cout << "[Network] Sent flap for player " << playerId << std::endl;
    };

    // Vérifier si des paquets sont disponibles
    networkTable["hasPackets"] = [client]() -> bool {
        return client->hasReceivedPackets();
    };

    // Recevoir le prochain paquet
    networkTable["getNextPacket"] = [client]() -> sol::optional<sol::table> {
        if (!client->hasReceivedPackets()) {
            return sol::nullopt;
        }

        NetworkPacket packet = client->getNextReceivedPacket();
        
        // Créer une table Lua pour le paquet
        auto& lua = Scripting::LuaState::Instance().GetState();
        sol::table packetTable = lua.create_table();
        packetTable["type"] = packet.header.type;
        packetTable["size"] = packet.payload.size();
        
        // Copier les données du payload dans un array Lua
        sol::table dataTable = lua.create_table();
        for (size_t i = 0; i < packet.payload.size(); ++i) {
            dataTable[i + 1] = static_cast<uint8_t>(packet.payload[i]);  // Lua arrays start at 1
        }
        packetTable["data"] = dataTable;
        
        return packetTable;
    };

    // Update du client réseau (à appeler chaque frame)
    networkTable["update"] = [client](float dt) {
        client->update(dt);
        client->process();
    };

    // Helper: Convert 4 bytes to float (little-endian)
    networkTable["bytesToFloat"] = [](sol::table bytes) -> float {
        if (bytes.size() != 4) {
            std::cerr << "[NetworkBindings] bytesToFloat requires exactly 4 bytes" << std::endl;
            return 0.0f;
        }
        
        // Extract bytes (Lua tables are 1-indexed)
        uint8_t b[4];
        b[0] = bytes[1];
        b[1] = bytes[2];
        b[2] = bytes[3];
        b[3] = bytes[4];
        
        // Reinterpret as float
        float result;
        std::memcpy(&result, b, sizeof(float));
        return result;
    };

    std::cout << "[NetworkBindings] NetworkClient bindings registered" << std::endl;
}

} // namespace FlappyBird
