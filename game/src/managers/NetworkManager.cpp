/**
 * NetworkManager.cpp - Network Manager Implementation (Phase 5)
 */

#include "managers/NetworkManager.hpp"
#include <iostream>

bool NetworkManager::initialize()
{
    std::cout << "[NetworkManager] 🌐 Initializing network subsystem..." << std::endl;
    // TODO Phase 5.3: Initialize ASIO
    std::cout << "[NetworkManager] ✅ Network initialized (simulated)" << std::endl;
    return true;
}

void NetworkManager::shutdown()
{
    std::cout << "[NetworkManager] Shutting down network..." << std::endl;
    disconnect();
    stopServer();
}

// === Client Functions ===

bool NetworkManager::connectToServer(const std::string& address, unsigned short port, const std::string& playerName)
{
    std::cout << "[NetworkManager] 🔌 Connecting to " << address << ":" << port 
              << " as '" << playerName << "'..." << std::endl;
    
    playerName_ = playerName;
    
    // TODO Phase 5.3: Actual network connection with ASIO
    // For now, simulate connection
    connected_ = true;
    clientId_ = 12345; // Simulated ID
    
    std::cout << "[NetworkManager] ✅ Connected! Client ID: " << clientId_ << std::endl;
    
    if (onConnection_) {
        onConnection_(true, "Connected successfully");
    }
    
    return true;
}

void NetworkManager::disconnect()
{
    if (connected_)
    {
        std::cout << "[NetworkManager] Disconnecting from server..." << std::endl;
        connected_ = false;
        clientId_ = 0;
        currentRoomId_ = 0;
        
        if (onConnection_) {
            onConnection_(false, "Disconnected");
        }
    }
}

// === Server Functions ===

bool NetworkManager::startServer(unsigned short port, uint8_t maxPlayers)
{
    std::cout << "[NetworkManager] 🎮 Starting server on port " << port 
              << " (max " << static_cast<int>(maxPlayers) << " players)..." << std::endl;
    
    // TODO Phase 5.3: Start actual ASIO server
    hosting_ = true;
    connected_ = true; // Host is also a client
    clientId_ = 1; // Host is always ID 1
    
    std::cout << "[NetworkManager] ✅ Server started!" << std::endl;
    return true;
}

void NetworkManager::stopServer()
{
    if (hosting_)
    {
        std::cout << "[NetworkManager] Stopping server..." << std::endl;
        hosting_ = false;
        connected_ = false;
    }
}

// === Lobby Functions ===

void NetworkManager::requestRoomList()
{
    std::cout << "[NetworkManager] 🔍 Requesting room list..." << std::endl;
    
    // TODO Phase 5.3: Send LOBBY_LIST_REQUEST packet
    
    // Simulate response with fake rooms
    roomList_.clear();
    
    Network::RoomInfo room1;
    room1.roomId = 1;
    std::strncpy(room1.roomName, "Bob's Room", sizeof(room1.roomName));
    room1.currentPlayers = 2;
    room1.maxPlayers = 4;
    room1.inGame = false;
    roomList_.push_back(room1);
    
    Network::RoomInfo room2;
    room2.roomId = 2;
    std::strncpy(room2.roomName, "Pro Players Only", sizeof(room2.roomName));
    room2.currentPlayers = 3;
    room2.maxPlayers = 4;
    room2.inGame = false;
    roomList_.push_back(room2);
    
    Network::RoomInfo room3;
    room3.roomId = 3;
    std::strncpy(room3.roomName, "Casual Game", sizeof(room3.roomName));
    room3.currentPlayers = 1;
    room3.maxPlayers = 2;
    room3.inGame = true; // In progress
    roomList_.push_back(room3);
    
    std::cout << "[NetworkManager] ✅ Found " << roomList_.size() << " rooms" << std::endl;
    
    if (onRoomList_) {
        onRoomList_(roomList_);
    }
}

void NetworkManager::createRoom(const std::string& roomName, uint8_t maxPlayers)
{
    std::cout << "[NetworkManager] 🏠 Creating room '" << roomName << "' (max " 
              << static_cast<int>(maxPlayers) << " players)..." << std::endl;
    
    // TODO Phase 5.3: Send ROOM_CREATE packet
    
    currentRoomId_ = 1; // Simulated room ID
    std::cout << "[NetworkManager] ✅ Room created! ID: " << currentRoomId_ << std::endl;
}

void NetworkManager::joinRoom(uint32_t roomId)
{
    std::cout << "[NetworkManager] 🚪 Joining room " << roomId << "..." << std::endl;
    
    // TODO Phase 5.3: Send ROOM_JOIN packet
    
    currentRoomId_ = roomId;
    std::cout << "[NetworkManager] ✅ Joined room " << roomId << std::endl;
}

void NetworkManager::leaveRoom()
{
    if (currentRoomId_ != 0)
    {
        std::cout << "[NetworkManager] 🚪 Leaving room " << currentRoomId_ << "..." << std::endl;
        
        // TODO Phase 5.3: Send ROOM_LEAVE packet
        
        currentRoomId_ = 0;
        std::cout << "[NetworkManager] ✅ Left room" << std::endl;
    }
}

void NetworkManager::setReady(bool ready)
{
    std::cout << "[NetworkManager] " << (ready ? "✅ Ready!" : "⏸️ Not ready") << std::endl;
    
    // TODO Phase 5.3: Send PLAYER_READY/PLAYER_NOT_READY packet
}

void NetworkManager::update()
{
    // TODO Phase 5.3: Process incoming packets
    // - Handle server responses
    // - Update room list
    // - Update game state
}
