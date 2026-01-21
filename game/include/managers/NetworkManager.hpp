/**
 * NetworkManager.hpp - Network Manager (Client & Server)
 * 
 * Manages network connections, hosting, and communication.
 * Phase 5: Network & Multiplayer
 */

#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include "network/GamePackets.hpp"

class NetworkManager
{
public:
    NetworkManager() = default;
    ~NetworkManager() = default;

    /**
     * Initialize network subsystem
     */
    bool initialize();

    /**
     * Shutdown network subsystem
     */
    void shutdown();

    // === Client Functions ===
    
    /**
     * Connect to server as client
     */
    bool connectToServer(const std::string& address, unsigned short port, const std::string& playerName);

    /**
     * Disconnect from server
     */
    void disconnect();

    /**
     * Check if connected to a server
     */
    bool isConnected() const { return connected_; }

    // === Server Functions ===
    
    /**
     * Start hosting a game server
     */
    bool startServer(unsigned short port, uint8_t maxPlayers);

    /**
     * Stop hosting server
     */
    void stopServer();

    /**
     * Check if hosting
     */
    bool isHosting() const { return hosting_; }

    // === Lobby Functions ===
    
    /**
     * Request list of available rooms (client)
     */
    void requestRoomList();

    /**
     * Create a room (when hosting)
     */
    void createRoom(const std::string& roomName, uint8_t maxPlayers);

    /**
     * Join a room (client)
     */
    void joinRoom(uint32_t roomId);

    /**
     * Leave current room
     */
    void leaveRoom();

    /**
     * Mark player as ready
     */
    void setReady(bool ready);

    // === Data Access ===
    
    /**
     * Get current room list
     */
    const std::vector<Network::RoomInfo>& getRoomList() const { return roomList_; }

    /**
     * Get current room ID
     */
    uint32_t getCurrentRoomId() const { return currentRoomId_; }

    /**
     * Get player name
     */
    const std::string& getPlayerName() const { return playerName_; }

    // === Callbacks ===
    
    using ConnectionCallback = std::function<void(bool success, const std::string& message)>;
    using RoomListCallback = std::function<void(const std::vector<Network::RoomInfo>&)>;
    using RoomUpdateCallback = std::function<void(const Network::RoomInfo&)>;
    
    void setConnectionCallback(ConnectionCallback callback) { onConnection_ = callback; }
    void setRoomListCallback(RoomListCallback callback) { onRoomList_ = callback; }
    void setRoomUpdateCallback(RoomUpdateCallback callback) { onRoomUpdate_ = callback; }

    /**
     * Update network (process packets)
     */
    void update();

private:
    // Connection state
    bool connected_ = false;
    bool hosting_ = false;
    uint32_t clientId_ = 0;
    std::string playerName_ = "Player";
    
    // Room state
    uint32_t currentRoomId_ = 0;
    std::vector<Network::RoomInfo> roomList_;
    
    // Callbacks
    ConnectionCallback onConnection_;
    RoomListCallback onRoomList_;
    RoomUpdateCallback onRoomUpdate_;
    
    // TODO Phase 5.3: Add actual network client/server
    // std::unique_ptr<NetworkClient> client_;
    // std::unique_ptr<NetworkServer> server_;
};
