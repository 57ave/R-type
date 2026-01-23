/**
 * NetworkManager.hpp - Network Manager (Client & Server)
 * 
 * Manages network connections, hosting, and communication.
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     * Phase 5.3: Real ASIO Network Integration
 */

#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include "network/GamePackets.hpp"
#include "network/NetworkClient.hpp"

// Player information in a room
struct PlayerInfo {
    uint32_t playerId;
    std::string playerName;
    bool isHost;
    bool isReady;
};

class NetworkManager
{
public:
    NetworkManager() = default;
    ~NetworkManager();

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

    /**
     * Start game (host only)
     */
    void startGame();

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
     * Get local player ID (assigned by server)
     */
    uint32_t getLocalPlayerId() const { return clientId_; }

    /**
     * Get players in current room
     */
    const std::vector<PlayerInfo>& getRoomPlayers() const { return roomPlayers_; }

    /**
     * Get player name
     */
    const std::string& getPlayerName() const { return playerName_; }

    // === Callbacks ===
    
    using ConnectionCallback = std::function<void(bool success, const std::string& message)>;
    using RoomListCallback = std::function<void(const std::vector<Network::RoomInfo>&)>;
    using RoomUpdateCallback = std::function<void(const Network::RoomInfo&)>;
    using GameStartCallback = std::function<void()>;
    
    void setConnectionCallback(ConnectionCallback callback) { onConnection_ = callback; }
    void setRoomListCallback(RoomListCallback callback) { onRoomList_ = callback; }
    void setRoomUpdateCallback(RoomUpdateCallback callback) { onRoomUpdate_ = callback; }
    void setGameStartCallback(GameStartCallback callback) { gameStartCallback_ = callback; }

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
    std::vector<PlayerInfo> roomPlayers_;  // Players in current room
    
    // Callbacks
    ConnectionCallback onConnection_;
    RoomListCallback onRoomList_;
    RoomUpdateCallback onRoomUpdate_;
    GameStartCallback gameStartCallback_;
    
    // Phase 5.3: Real network client
    std::unique_ptr<NetworkClient> client_;
    
    // Server info (when hosting)
    std::string serverAddress_;
    unsigned short serverPort_ = 0;
    
    // Helper methods
    void processIncomingPackets();
    void handlePacket(const char* data, size_t length);
};
