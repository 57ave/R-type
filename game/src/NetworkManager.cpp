#include "network/NetworkManager.hpp"
#include <network/NetworkClient.hpp>
#include <network/Packet.hpp>
#include <network/RTypeProtocol.hpp>
#include <iostream>
#include <thread>
#include <chrono>

namespace RType {
namespace Game {

NetworkManager::NetworkManager()
    : networkClient_(nullptr)
    , connected_(false)
    , currentRoomId_(0)
    , myPlayerId_(0)
{
    std::cout << "[NetworkManager] Initialized" << std::endl;
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::connect(const std::string& serverIp, uint16_t port) {
    try {
        // If already connected, disconnect first to avoid socket conflicts
        if (connected_ || networkClient_) {
            std::cout << "[NetworkManager] Already connected, disconnecting first..." << std::endl;
            disconnect();
            // Give the system time to clean up
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "[NetworkManager] Connecting to " << serverIp << ":" << port << std::endl;
        
        networkClient_ = std::make_unique<NetworkClient>(serverIp, port);
        networkClient_->start();
        
        // Send CLIENT_HELLO to establish connection
        NetworkPacket helloPacket(static_cast<uint16_t>(GamePacketType::CLIENT_HELLO));
        networkClient_->sendPacket(helloPacket);
        
        connected_ = true;
        std::cout << "[NetworkManager] Connected successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[NetworkManager] Connection failed: " << e.what() << std::endl;
        connected_ = false;
        return false;
    }
}

void NetworkManager::disconnect() {
    if (!connected_) return;
    
    std::cout << "[NetworkManager] Disconnecting..." << std::endl;
    
    // Leave current room if in one
    if (currentRoomId_ != 0) {
        leaveRoom();
    }
    
    // Send disconnect packet
    if (networkClient_) {
        NetworkPacket disconnectPacket(static_cast<uint16_t>(GamePacketType::CLIENT_DISCONNECT));
        networkClient_->sendPacket(disconnectPacket);
        networkClient_.reset();
    }
    
    connected_ = false;
    currentRoomId_ = 0;
    myPlayerId_ = 0;
    
    std::cout << "[NetworkManager] Disconnected" << std::endl;
}

bool NetworkManager::isConnected() const {
    return connected_;
}

void NetworkManager::requestRoomList() {
    if (!connected_ || !networkClient_) {
        std::cerr << "[NetworkManager] Cannot request room list: not connected" << std::endl;
        return;
    }
    
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ROOM_LIST));
    networkClient_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Requested room list" << std::endl;
}

void NetworkManager::createRoom(const std::string& name, uint8_t maxPlayers, uint8_t difficulty, const std::string& password) {
    if (!connected_ || !networkClient_) {
        std::cerr << "[NetworkManager] Cannot create room: not connected" << std::endl;
        return;
    }
    
    CreateRoomPayload payload;
    payload.name = name;
    payload.maxPlayers = maxPlayers;
    // difficulty and password not supported in current protocol
    
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CREATE_ROOM));
    packet.setPayload(payload.serialize());
    networkClient_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Created room '" << name << "'" << std::endl;
}

void NetworkManager::joinRoom(uint32_t roomId, const std::string& password) {
    if (!connected_ || !networkClient_) {
        std::cerr << "[NetworkManager] Cannot join room: not connected" << std::endl;
        return;
    }
    
    JoinRoomPayload payload;
    payload.roomId = roomId;
    // password not supported in current protocol
    
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::JOIN_ROOM));
    packet.setPayload(payload.serialize());
    networkClient_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Joining room " << roomId << std::endl;
}

void NetworkManager::leaveRoom() {
    // LEAVE_ROOM not implemented in protocol yet
    std::cout << "[NetworkManager] leaveRoom() - not implemented" << std::endl;
    currentRoomId_ = 0;
}

void NetworkManager::setReady(bool ready) {
    // PLAYER_READY not implemented in protocol yet
    std::cout << "[NetworkManager] setReady(" << ready << ") - not implemented" << std::endl;
}

void NetworkManager::sendChatMessage(const std::string& message) {
    // CHAT_MESSAGE not implemented in protocol yet
    std::cout << "[NetworkManager] sendChatMessage('" << message << "') - not implemented" << std::endl;
}

void NetworkManager::startGame() {
    if (!connected_ || !networkClient_ || currentRoomId_ == 0) {
        std::cerr << "[NetworkManager] Cannot start game: not in a room" << std::endl;
        return;
    }
    
    NetworkPacket packet(static_cast<uint16_t>(GamePacketType::GAME_START));
    Network::Serializer serializer;
    serializer.write(currentRoomId_);
    packet.setPayload(serializer.getBuffer());
    networkClient_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Requested game start" << std::endl;
}

void NetworkManager::update() {
    if (!connected_ || !networkClient_) return;
    
    // Process network
    networkClient_->process();
    
    // Handle incoming packets
    processPackets();
}

void NetworkManager::processPackets() {
    if (!networkClient_) return;
    
    while (networkClient_->hasReceivedPackets()) {
        NetworkPacket packet = networkClient_->getNextReceivedPacket();
        auto type = static_cast<GamePacketType>(packet.header.type);
        
        switch (type) {
            case GamePacketType::SERVER_WELCOME:
                if (!packet.payload.empty()) {
                    myPlayerId_ = packet.payload[0];
                    std::cout << "[NetworkManager] Received player ID: " << (int)myPlayerId_ << std::endl;
                }
                break;
                
            case GamePacketType::ROOM_LIST_REPLY:
                handleRoomListReply(packet);
                break;
                
            case GamePacketType::ROOM_CREATED:
                handleRoomCreated(packet);
                break;
                
            case GamePacketType::ROOM_JOINED:
                handleRoomJoined(packet);
                break;
                
            case GamePacketType::GAME_START:
                handleGameStart(packet);
                break;
                
            default:
                // Other packets (WORLD_SNAPSHOT, etc.) are handled elsewhere
                break;
        }
    }
}

void NetworkManager::handleRoomListReply(const NetworkPacket& packet) {
    if (packet.payload.empty()) return;
    
    try {
        RoomListPayload payload = RoomListPayload::deserialize(packet.payload);
        
        std::cout << "[NetworkManager] Received room list with " << payload.rooms.size() << " rooms" << std::endl;
        
        if (onRoomListReceived_) {
            onRoomListReceived_(payload.rooms);
        }
    } catch (const std::exception& e) {
        std::cerr << "[NetworkManager] Error parsing room list: " << e.what() << std::endl;
    }
}

void NetworkManager::handleRoomCreated(const NetworkPacket& packet) {
    if (packet.payload.size() < sizeof(uint32_t)) {
        std::cerr << "[NetworkManager] ROOM_CREATED payload too small" << std::endl;
        return;
    }
    
    try {
        Network::Deserializer deserializer(packet.payload);
        uint32_t roomId = deserializer.read<uint32_t>();
        
        std::cout << "[NetworkManager] Room created with ID: " << roomId << std::endl;
        
        currentRoomId_ = roomId;
        
        if (onRoomCreated_) {
            onRoomCreated_(roomId, true, "");
        }
    } catch (const std::exception& e) {
        std::cerr << "[NetworkManager] Error parsing room created: " << e.what() << std::endl;
    }
}

void NetworkManager::handleRoomJoined(const NetworkPacket& packet) {
    if (packet.payload.size() < sizeof(uint32_t)) {
        std::cerr << "[NetworkManager] ROOM_JOINED payload too small" << std::endl;
        return;
    }
    
    try {
        Network::Deserializer deserializer(packet.payload);
        uint32_t roomId = deserializer.read<uint32_t>();
        std::string roomName = deserializer.readString();
        
        std::cout << "[NetworkManager] Joined room " << roomId << ": " << roomName << std::endl;
        
        currentRoomId_ = roomId;
        
        if (onRoomJoined_) {
            onRoomJoined_(roomId, true, "");
        }
    } catch (const std::exception& e) {
        std::cerr << "[NetworkManager] Error parsing room joined: " << e.what() << std::endl;
    }
}

void NetworkManager::handlePlayerReadyUpdate(const NetworkPacket&) {
    // PLAYER_READY_UPDATE not implemented in protocol
    std::cout << "[NetworkManager] handlePlayerReadyUpdate() - not implemented" << std::endl;
}

void NetworkManager::handleGameStart(const NetworkPacket& packet) {
    std::cout << "[NetworkManager] Game is starting!" << std::endl;
    
    if (onGameStart_) {
        onGameStart_();
    }
}

void NetworkManager::handleChatMessage(const NetworkPacket&) {
    // CHAT_MESSAGE not implemented in protocol
    std::cout << "[NetworkManager] handleChatMessage() - not implemented" << std::endl;
}

// Callback setters
void NetworkManager::setRoomListCallback(RoomListCallback callback) {
    onRoomListReceived_ = callback;
}

void NetworkManager::setRoomCreatedCallback(RoomCreatedCallback callback) {
    onRoomCreated_ = callback;
}

void NetworkManager::setRoomJoinedCallback(RoomJoinedCallback callback) {
    onRoomJoined_ = callback;
}

void NetworkManager::setChatMessageCallback(ChatMessageCallback callback) {
    onChatMessage_ = callback;
}

void NetworkManager::setPlayerReadyCallback(PlayerReadyCallback callback) {
    onPlayerReady_ = callback;
}

void NetworkManager::setGameStartCallback(GameStartCallback callback) {
    onGameStart_ = callback;
}

} // namespace Game
} // namespace RType
