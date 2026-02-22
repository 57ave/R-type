/**
 * NetworkManager.hpp - Network Manager (Client & Server)
 * 
 * Manages network connections, hosting, and communication.
 * Phase 5.3: Real ASIO Network Integration
 * Phase 6.5: Gameplay synchronization (WORLD_SNAPSHOT + CLIENT_INPUT)
 */

#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include "network/GamePackets.hpp"
#include "network/NetworkClient.hpp"
#include "network/RTypeProtocol.hpp"

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

    // === Gameplay Functions (Phase 6.5) ===
    
    /**
     * Send player input to server
     * @param up Moving up
     * @param down Moving down
     * @param left Moving left
     * @param right Moving right
     * @param fire Shooting
     * @param chargeLevel Charge level (0-5)
     */
    void sendInput(bool up, bool down, bool left, bool right, bool fire, uint8_t chargeLevel = 0);

    /**
     * Check if game is in multiplayer mode (in a room with game started)
     */
    bool isInGame() const { return inGame_; }

    // === Data Access ===
    
    /**
     * Get current room list
     */
    const std::vector<Network::RoomInfo>& getRoomList() const { return roomList_; }

    /**
     * Get room list version (incremented each time room list is updated)
     */
    uint32_t getRoomListVersion() const { return roomListVersion_; }

    /**
     * Get current room ID
     */
    uint32_t getCurrentRoomId() const { return currentRoomId_; }

    /**
     * Get current room name
     */
    const std::string& getCurrentRoomName() const { return currentRoomName_; }

    /**
     * Get current room max players
     */
    uint8_t getCurrentMaxPlayers() const { return currentMaxPlayers_; }

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

    // === Chat ===

    /**
     * Send a chat message to the current room
     */
    void sendChatMessage(const std::string& message);

    /**
     * Chat message struct for display
     */
    struct ChatMessage {
        std::string senderName;
        std::string message;
    };

    /**
     * Get chat messages for display
     */
    const std::vector<ChatMessage>& getChatMessages() const { return chatMessages_; }

    /**
     * Get chat version (incremented on each new message)
     */
    uint32_t getChatVersion() const { return chatVersion_; }

    // === Lag Compensation ===

    /**
     * Get current input sequence number (last sent)
     */
    uint32_t getInputSequence() const { return inputSequence_; }

    /**
     * Get smoothed round-trip time in seconds
     */
    float getSmoothedRtt() const { return smoothedRtt_; }

    /**
     * Get raw round-trip time in seconds
     */
    float getRtt() const { return rtt_; }

    // === Callbacks ===

    using ConnectionCallback = std::function<void(bool success, const std::string& message)>;
    using RoomListCallback = std::function<void(const std::vector<Network::RoomInfo>&)>;
    using RoomUpdateCallback = std::function<void(const Network::RoomInfo&)>;
    using GameStartCallback = std::function<void()>;
    using WorldSnapshotCallback = std::function<void(const RType::WorldSnapshotData&)>;
    using LevelChangeCallback = std::function<void(uint8_t level)>;
    using GameOverCallback = std::function<void(uint32_t totalScore)>;
    using VictoryCallback = std::function<void(uint32_t totalScore)>;

    void setConnectionCallback(ConnectionCallback callback) { onConnection_ = callback; }
    void setRoomListCallback(RoomListCallback callback) { onRoomList_ = callback; }
    void setRoomUpdateCallback(RoomUpdateCallback callback) { onRoomUpdate_ = callback; }
    void setGameStartCallback(GameStartCallback callback) { gameStartCallback_ = callback; }
    void setWorldSnapshotCallback(WorldSnapshotCallback callback) { onWorldSnapshot_ = callback; }
    void setLevelChangeCallback(LevelChangeCallback callback) { onLevelChange_ = callback; }
    void setGameOverCallback(GameOverCallback callback) { onGameOver_ = callback; }
    void setVictoryCallback(VictoryCallback callback) { onVictory_ = callback; }

    /**
     * Update network (process packets, send periodic pings)
     * @param deltaTime Frame delta time in seconds (for ping timer)
     */
    void update(float deltaTime = 0.0f);

private:
    // Connection state
    bool connected_ = false;
    bool hosting_ = false;
    bool inGame_ = false;  // True when game has started
    uint32_t clientId_ = 0;
    std::string playerName_ = "Player";
    
    // Room state
    uint32_t currentRoomId_ = 0;
    std::string currentRoomName_;
    uint8_t currentMaxPlayers_ = 4;
    std::vector<Network::RoomInfo> roomList_;
    uint32_t roomListVersion_ = 0;  // Incremented each time room list is updated
    std::vector<PlayerInfo> roomPlayers_;  // Players in current room
    
    // Chat state
    std::vector<ChatMessage> chatMessages_;
    uint32_t chatVersion_ = 0;
    
    // Callbacks
    ConnectionCallback onConnection_;
    RoomListCallback onRoomList_;
    RoomUpdateCallback onRoomUpdate_;
    GameStartCallback gameStartCallback_;
    WorldSnapshotCallback onWorldSnapshot_;
    LevelChangeCallback onLevelChange_;
    GameOverCallback onGameOver_;
    VictoryCallback onVictory_;
    
    // Phase 5.3: Real network client
    std::unique_ptr<NetworkClient> client_;

    // Server info (when hosting)
    std::string serverAddress_;
    unsigned short serverPort_ = 0;

    // Lag compensation state
    uint32_t inputSequence_ = 0;       // Monotonic input counter
    uint32_t lastSnapshotSeq_ = 0;     // Last received snapshot seq (for ordering)
    float rtt_ = 0.0f;                 // Raw RTT in seconds
    float smoothedRtt_ = 0.0f;         // Exponential moving average RTT
    float pingTimer_ = 0.0f;           // Timer for periodic pings
    uint32_t lastPingTimestamp_ = 0;    // Timestamp sent in last ping

    // Helper methods
    void processIncomingPackets();
    void handlePacket(const char* data, size_t length);
    void sendPing();
};
