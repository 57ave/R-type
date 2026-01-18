#pragma once

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>

class NetworkClient;
struct NetworkPacket;
struct RoomInfo;

namespace RType {
namespace Game {

class NetworkManager {
public:
    // Callback types
    using RoomListCallback = std::function<void(const std::vector<RoomInfo>&)>;
    using RoomCreatedCallback = std::function<void(uint32_t roomId, bool success, const std::string& errorMsg)>;
    using RoomJoinedCallback = std::function<void(uint32_t roomId, bool success, const std::string& errorMsg)>;
    using ChatMessageCallback = std::function<void(const std::string& sender, const std::string& message)>;
    using PlayerReadyCallback = std::function<void(uint32_t playerId, bool ready)>;
    using GameStartCallback = std::function<void()>;

    NetworkManager();
    ~NetworkManager();

    // Connection management
    bool connect(const std::string& serverIp, uint16_t port);
    void disconnect();
    bool isConnected() const;

    // Room management
    void requestRoomList();
    void createRoom(const std::string& name, uint8_t maxPlayers, uint8_t difficulty = 1, const std::string& password = "");
    void joinRoom(uint32_t roomId, const std::string& password = "");
    void leaveRoom();
    void setReady(bool ready);
    void sendChatMessage(const std::string& message);
    void startGame();

    // Update
    void update();

    // Callback setters
    void setRoomListCallback(RoomListCallback callback);
    void setRoomCreatedCallback(RoomCreatedCallback callback);
    void setRoomJoinedCallback(RoomJoinedCallback callback);
    void setChatMessageCallback(ChatMessageCallback callback);
    void setPlayerReadyCallback(PlayerReadyCallback callback);
    void setGameStartCallback(GameStartCallback callback);

    // Getters
    uint32_t getCurrentRoomId() const { return currentRoomId_; }
    uint8_t getMyPlayerId() const { return myPlayerId_; }

private:
    void processPackets();
    void handleRoomListReply(const NetworkPacket& packet);
    void handleRoomCreated(const NetworkPacket& packet);
    void handleRoomJoined(const NetworkPacket& packet);
    void handlePlayerReadyUpdate(const NetworkPacket& packet);
    void handleGameStart(const NetworkPacket& packet);
    void handleChatMessage(const NetworkPacket& packet);

    std::unique_ptr<NetworkClient> networkClient_;
    bool connected_;
    uint32_t currentRoomId_;
    uint8_t myPlayerId_;

    // Callbacks
    RoomListCallback onRoomListReceived_;
    RoomCreatedCallback onRoomCreated_;
    RoomJoinedCallback onRoomJoined_;
    ChatMessageCallback onChatMessage_;
    PlayerReadyCallback onPlayerReady_;
    GameStartCallback onGameStart_;
};

} // namespace Game
} // namespace RType
