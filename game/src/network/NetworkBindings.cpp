#include "network/NetworkBindings.hpp"

#include <algorithm>
#include <iostream>

namespace RType {
namespace Network {

// Static members initialization
std::shared_ptr<NetworkClient> NetworkBindings::s_networkClient = nullptr;
sol::state* NetworkBindings::s_lua = nullptr;

void NetworkBindings::RegisterAll(sol::state& lua) {
    s_lua = &lua;

    // Create the Network table in Lua
    auto netTable = lua["Network"].get_or_create<sol::table>();

    // Register functions callable from Lua
    netTable["RequestRoomList"] = &NetworkBindings::RequestRoomList;
    netTable["CreateRoom"] = &NetworkBindings::CreateRoom;
    netTable["JoinRoom"] = &NetworkBindings::JoinRoom;
    netTable["Connect"] = &NetworkBindings::Connect;
    netTable["LeaveRoom"] = &NetworkBindings::LeaveRoom;
    netTable["SetPlayerReady"] = &NetworkBindings::SetPlayerReady;
    netTable["StartGame"] = &NetworkBindings::StartGame;
    netTable["SendChatMessage"] = &NetworkBindings::SendChatMessage;

    std::cout << "[NetworkBindings] Network functions registered to Lua" << std::endl;
}

void NetworkBindings::SetNetworkClient(std::shared_ptr<NetworkClient> client) {
    s_networkClient = client;
    std::cout << "[NetworkBindings] NetworkClient set" << std::endl;
}

std::shared_ptr<NetworkClient> NetworkBindings::GetNetworkClient() {
    return s_networkClient;
}

// ========== Functions called FROM Lua ==========

void NetworkBindings::RequestRoomList() {
    if (!s_networkClient) {
        std::cerr << "[NetworkBindings] Cannot request room list: NetworkClient not set"
                  << std::endl;
        return;
    }

    if (!s_networkClient->isConnected()) {
        std::cerr << "[NetworkBindings] Cannot request room list: not connected to server"
                  << std::endl;
        return;
    }

    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ROOM_LIST));
    s_networkClient->sendPacket(packet);

    std::cout << "[NetworkBindings] ROOM_LIST request sent to server" << std::endl;
}

void NetworkBindings::Connect(const std::string& host, int port) {
    try {
        if (host.empty() || port <= 0) {
            std::cerr << "[NetworkBindings] Invalid host/port" << std::endl;
            return;
        }

        // If we already have an active connected client, do not open another connection.
        if (s_networkClient && s_networkClient->isConnected()) {
            std::cout << "[NetworkBindings] Already connected - ignoring duplicate Connect()"
                      << std::endl;
            // Notify Lua that we're connected (useful if UI initiated a redundant call)
            if (s_lua) {
                sol::protected_function onConnected = (*s_lua)["OnConnected"];
                if (onConnected.valid()) {
                    try {
                        onConnected(host, port);
                    } catch (const std::exception& e) {
                        std::cerr << "[NetworkBindings] Lua OnConnected error: " << e.what()
                                  << std::endl;
                    }
                }
            }
            return;
        }

        // If a client exists but is not connected, attempt to cleanly disconnect and reset it
        if (s_networkClient) {
            try {
                s_networkClient->disconnect();
            } catch (const std::exception& e) {
                std::cerr
                    << "[NetworkBindings] Warning: error while disconnecting previous client: "
                    << e.what() << std::endl;
            }
            s_networkClient.reset();
        }

        auto client = std::make_shared<NetworkClient>(host, static_cast<short>(port));
        client->start();
        // Send CLIENT_HELLO so the server recognizes and replies with Welcome
        try {
            client->sendHello();
        } catch (const std::exception& e) {
            std::cerr << "[NetworkBindings] Warning: sendHello() threw: " << e.what() << std::endl;
        }
        SetNetworkClient(client);

        std::cout << "[NetworkBindings] Connected to " << host << ":" << port << std::endl;

        // Notify Lua that we've connected (optional callback)
        if (s_lua) {
            try {
                sol::protected_function onConnected = (*s_lua)["OnConnected"];
                if (onConnected.valid()) {
                    auto res = onConnected(host, port);
                    if (!res.valid()) {
                        sol::error err = res;
                        std::cerr << "[NetworkBindings] Lua OnConnected error: " << err.what()
                                  << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[NetworkBindings] Exception calling OnConnected: " << e.what()
                          << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[NetworkBindings] Connect error: " << e.what() << std::endl;
    }
}

void NetworkBindings::CreateRoom(const std::string& name, int maxPlayers,
                                 const std::string& password, int difficulty) {
    if (!s_networkClient) {
        std::cerr << "[NetworkBindings] Cannot create room: NetworkClient not set" << std::endl;
        return;
    }

    if (!s_networkClient->isConnected()) {
        std::cerr << "[NetworkBindings] Cannot create room: not connected to server" << std::endl;
        return;
    }

    // Prepare payload
    CreateRoomPayload payload;
    payload.name = name.empty() ? "New Room" : name;
    payload.maxPlayers = static_cast<uint8_t>(std::clamp(maxPlayers, 2, 8));

    // Create packet
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CREATE_ROOM));
    packet.setPayload(payload.serialize());
    s_networkClient->sendPacket(packet);

    std::cout << "[NetworkBindings] CREATE_ROOM sent: '" << payload.name
              << "' (max: " << static_cast<int>(payload.maxPlayers) << " players)" << std::endl;
}

void NetworkBindings::JoinRoom(uint32_t roomId) {
    if (!s_networkClient) {
        std::cerr << "[NetworkBindings] Cannot join room: NetworkClient not set" << std::endl;
        return;
    }

    if (!s_networkClient->isConnected()) {
        std::cerr << "[NetworkBindings] Cannot join room: not connected to server" << std::endl;
        return;
    }

    // Prepare payload
    JoinRoomPayload payload;
    payload.roomId = roomId;

    // Create packet
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::JOIN_ROOM));
    packet.setPayload(payload.serialize());
    s_networkClient->sendPacket(packet);

    std::cout << "[NetworkBindings] JOIN_ROOM sent: roomId=" << roomId << std::endl;
}

void NetworkBindings::LeaveRoom() {
    if (!s_networkClient) {
        std::cerr << "[NetworkBindings] Cannot leave room: NetworkClient not set" << std::endl;
        return;
    }

    if (!s_networkClient->isConnected()) {
        std::cerr << "[NetworkBindings] Cannot leave room: not connected to server" << std::endl;
        return;
    }

    // TODO: Define LEAVE_ROOM packet in protocol
    std::cout << "[NetworkBindings] LEAVE_ROOM (packet not implemented in protocol yet)"
              << std::endl;

    // For now, just disconnect and reconnect
    // NetworkPacket packet(static_cast<uint16_t>(GamePacketType::LEAVE_ROOM));
    // s_networkClient->sendPacket(packet);
}

void NetworkBindings::SetPlayerReady(bool ready) {
    if (!s_networkClient) {
        std::cerr << "[NetworkBindings] Cannot set ready: NetworkClient not set" << std::endl;
        return;
    }

    if (!s_networkClient->isConnected()) {
        std::cerr << "[NetworkBindings] Cannot set ready: not connected to server" << std::endl;
        return;
    }

    // TODO: Define PLAYER_READY packet in protocol
    std::cout << "[NetworkBindings] PLAYER_READY: " << (ready ? "true" : "false")
              << " (packet not implemented in protocol yet)" << std::endl;

    // NetworkPacket packet(static_cast<uint16_t>(GamePacketType::PLAYER_READY));
    // packet.setPayload(...);
    // s_networkClient->sendPacket(packet);
}

void NetworkBindings::StartGame() {
    if (!s_networkClient) {
        std::cerr << "[NetworkBindings] Cannot start game: NetworkClient not set" << std::endl;
        return;
    }

    if (!s_networkClient->isConnected()) {
        std::cerr << "[NetworkBindings] Cannot start game: not connected to server" << std::endl;
        return;
    }

    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::GAME_START));
    s_networkClient->sendPacket(packet);

    std::cout << "[NetworkBindings] GAME_START sent (host only)" << std::endl;
}

// ========== Callbacks called BY C++ when packets arrive ==========

void NetworkBindings::OnRoomListReceived(const std::vector<RoomInfo>& rooms) {
    if (!s_lua) {
        std::cerr << "[NetworkBindings] Cannot call Lua callback: Lua state not set" << std::endl;
        return;
    }

    // Get Lua callback function
    sol::protected_function luaCallback = (*s_lua)["OnServerListUpdated"];
    if (!luaCallback.valid()) {
        std::cerr << "[NetworkBindings] OnServerListUpdated not found in Lua" << std::endl;
        return;
    }

    // Convert vector<RoomInfo> to Lua table
    sol::table roomsTable = s_lua->create_table();
    for (size_t i = 0; i < rooms.size(); ++i) {
        sol::table roomTable = s_lua->create_table();
        roomTable["id"] = rooms[i].id;
        roomTable["name"] = rooms[i].name;
        roomTable["currentPlayers"] = rooms[i].currentPlayers;
        roomTable["maxPlayers"] = rooms[i].maxPlayers;
        roomsTable[i + 1] = roomTable;  // Lua tables start at index 1
    }

    // Call Lua function
    auto result = luaCallback(roomsTable);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Lua error in OnServerListUpdated: " << err.what()
                  << std::endl;
    } else {
        std::cout << "[NetworkBindings] Room list sent to Lua (" << rooms.size() << " rooms)"
                  << std::endl;
    }
}

void NetworkBindings::OnRoomCreated(uint32_t roomId) {
    std::cout << "[NetworkBindings] Room created with ID: " << roomId << std::endl;

    // NE PAS auto-join ici ! Le serveur le fait déjà dans handleCreateRoom
    // On attend juste la confirmation ROOM_JOINED du serveur
}

void NetworkBindings::OnRoomJoined(uint32_t roomId, const std::string& roomName, uint8_t maxPlayers,
                                   bool isHost) {
    if (!s_lua) {
        std::cerr << "[NetworkBindings] Cannot call Lua callback: Lua state not set" << std::endl;
        return;
    }

    // Get Lua callback function
    sol::protected_function luaCallback = (*s_lua)["OnRoomJoined"];
    if (!luaCallback.valid()) {
        std::cout << "[NetworkBindings] OnRoomJoined not found in Lua (optional callback)"
                  << std::endl;
        return;
    }

    // Create room info table with all properties
    sol::table roomInfo = s_lua->create_table();
    roomInfo["id"] = roomId;
    roomInfo["name"] = roomName;
    roomInfo["maxPlayers"] = maxPlayers;
    roomInfo["isHost"] = isHost;

    // Call Lua function
    auto result = luaCallback(roomInfo);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Lua error in OnRoomJoined: " << err.what() << std::endl;
    } else {
        std::cout << "[NetworkBindings] OnRoomJoined called in Lua (room: " << roomName
                  << ", max: " << static_cast<int>(maxPlayers)
                  << ", host: " << (isHost ? "YES" : "NO") << ")" << std::endl;
    }
}

void NetworkBindings::OnPlayerJoinedRoom(uint32_t playerId, const std::string& playerName) {
    if (!s_lua)
        return;

    sol::protected_function luaCallback = (*s_lua)["OnPlayerJoined"];
    if (!luaCallback.valid()) {
        std::cout << "[NetworkBindings] OnPlayerJoined not found in Lua (optional callback)"
                  << std::endl;
        return;
    }

    sol::table playerInfo = s_lua->create_table();
    playerInfo["id"] = playerId;
    playerInfo["name"] = playerName;
    playerInfo["ready"] = false;

    auto result = luaCallback(playerInfo);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Lua error in OnPlayerJoined: " << err.what() << std::endl;
    }
}

void NetworkBindings::OnPlayerLeftRoom(uint32_t playerId) {
    if (!s_lua)
        return;

    sol::protected_function luaCallback = (*s_lua)["OnPlayerLeft"];
    if (!luaCallback.valid()) {
        std::cout << "[NetworkBindings] OnPlayerLeft not found in Lua (optional callback)"
                  << std::endl;
        return;
    }

    auto result = luaCallback(playerId);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Lua error in OnPlayerLeft: " << err.what() << std::endl;
    }
}

void NetworkBindings::OnPlayerReadyChanged(uint32_t playerId, bool ready) {
    if (!s_lua)
        return;

    sol::protected_function luaCallback = (*s_lua)["OnPlayerReadyChanged"];
    if (!luaCallback.valid()) {
        std::cout << "[NetworkBindings] OnPlayerReadyChanged not found in Lua (optional callback)"
                  << std::endl;
        return;
    }

    auto result = luaCallback(playerId, ready);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Lua error in OnPlayerReadyChanged: " << err.what()
                  << std::endl;
    }
}

void NetworkBindings::OnGameStarting(int countdown) {
    if (!s_lua)
        return;

    sol::protected_function luaCallback = (*s_lua)["OnGameStarting"];
    if (!luaCallback.valid()) {
        std::cout << "[NetworkBindings] OnGameStarting not found in Lua (optional callback)"
                  << std::endl;
        return;
    }

    auto result = luaCallback(countdown);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Lua error in OnGameStarting: " << err.what() << std::endl;
    } else {
        std::cout << "[NetworkBindings] Game starting in " << countdown << " seconds" << std::endl;
    }
}

void NetworkBindings::OnRoomPlayersUpdated(const std::vector<PlayerInRoomInfo>& players) {
    if (!s_lua) {
        std::cerr << "[NetworkBindings] Cannot call OnRoomPlayersUpdated - Lua state not set"
                  << std::endl;
        return;
    }

    sol::protected_function luaCallback = (*s_lua)["OnRoomPlayersUpdated"];
    if (!luaCallback.valid()) {
        std::cerr << "[NetworkBindings] OnRoomPlayersUpdated not defined in Lua" << std::endl;
        return;
    }

    sol::table playersTable = s_lua->create_table();
    for (size_t i = 0; i < players.size(); ++i) {
        sol::table playerTable = s_lua->create_table();
        playerTable["id"] = players[i].playerId;
        playerTable["name"] = players[i].playerName;
        playerTable["isHost"] = players[i].isHost;
        playerTable["ready"] = players[i].isReady;
        playersTable[i + 1] = playerTable;
    }

    auto result = luaCallback(playersTable);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Error calling OnRoomPlayersUpdated: " << err.what()
                  << std::endl;
    } else {
        std::cout << "[NetworkBindings] Room players updated (" << players.size() << " players)"
                  << std::endl;
    }
}

void NetworkBindings::OnChatMessage(const std::string& senderName, const std::string& message) {
    if (!s_lua) return;
    
    std::cout << "[Client] RECEIVED CHAT: " << senderName << ": " << message << std::endl;

    sol::protected_function luaCallback = (*s_lua)["OnChatMessage"];
    if (!luaCallback.valid()) {
        std::cout << "[NetworkBindings] OnChatMessage not found in Lua (optional callback)"
                  << std::endl;
        return;
    }

    auto result = luaCallback(senderName, message);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[NetworkBindings] Lua error in OnChatMessage: " << err.what() << std::endl;
    }
}

void NetworkBindings::SendChatMessage(const std::string& message, uint32_t roomId) {
    if (!s_networkClient) {
        std::cerr << "[NetworkBindings] Cannot send chat message: NetworkClient not set"
                  << std::endl;
        return;
    }

    if (!s_networkClient->isConnected()) {
        std::cerr << "[NetworkBindings] Cannot send chat message: not connected to server"
                  << std::endl;
        return;
    }

    ChatMessagePayload payload;
    payload.senderId = 0;     // Will be set by server
    payload.senderName = "";  // Will be set by server
    payload.message = message;
    payload.roomId = roomId;
    
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CHAT_MESSAGE));
    packet.setPayload(payload.serialize());
    s_networkClient->sendPacket(packet);
    
    std::cout << "[NetworkBindings] Chat message sent: " << message << " (room " << roomId << ")" << std::endl;
}

} // namespace Network
} // namespace RType
