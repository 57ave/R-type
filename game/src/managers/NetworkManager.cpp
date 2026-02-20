/**
 * NetworkManager.cpp - Network Manager Implementation (Real ASIO)
 * Phase 5.3: Real network using engine's NetworkClient
 * Phase 6.5: Gameplay synchronization (WORLD_SNAPSHOT + CLIENT_INPUT)
 */

#include "managers/NetworkManager.hpp"
#include "network/Serializer.hpp"
#include "network/RTypeProtocol.hpp"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

NetworkManager::~NetworkManager() {
    shutdown();
}

bool NetworkManager::initialize() {
    std::cout << "[NetworkManager] Initialized" << std::endl;
    return true;
}

void NetworkManager::shutdown() {
    if (client_) {
        disconnect();
    }
    client_.reset();
}

bool NetworkManager::connectToServer(const std::string& address, uint16_t port, const std::string& playerName) {
    // Disconnect existing connection if any
    if (client_ && connected_) {
        std::cout << "[NetworkManager] Disconnecting existing connection..." << std::endl;
        disconnect();
    }

    serverAddress_ = address;
    serverPort_ = port;
    playerName_ = playerName;

    try {
        // Create NetworkClient - constructor connects automatically
        client_ = std::make_unique<NetworkClient>(address, port);
        client_->start();
        
        // Give io_context thread time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Send CLIENT_HELLO packet (uses CLIENT_CONNECT = 0x01 = CLIENT_HELLO)
        std::cout << "[NetworkManager] Sending CLIENT_HELLO to " << address << ":" << port << std::endl;
        client_->sendHello();
        
        connected_ = true;
        std::cout << "[NetworkManager] Connected to " << address << ":" << port << " as " << playerName << std::endl;
        
        if (onConnection_) {
            onConnection_(true, "Connected successfully");
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[NetworkManager] Connection error: " << e.what() << std::endl;
        client_.reset();
        connected_ = false;
        
        if (onConnection_) {
            onConnection_(false, e.what());
        }
        return false;
    }
}

void NetworkManager::disconnect() {
    // Debug: Print stack trace to find who called disconnect
    std::cout << "[NetworkManager] disconnect() called!" << std::endl;
    
    if (client_ && connected_) {
        // NetworkClient::disconnect() already sends CLIENT_DISCONNECT packet
        // So we just need to call it, not send our own packet
        client_->disconnect();
        std::cout << "[NetworkManager] Disconnected from server" << std::endl;
    }
    
    client_.reset();
    connected_ = false;
    hosting_ = false;
    currentRoomId_ = 0;
}

bool NetworkManager::startServer(uint16_t port, uint8_t maxPlayers) {
    std::cout << "[NetworkManager] Server should be started separately" << std::endl;
    std::cout << "[NetworkManager] Connecting to localhost:" << port << std::endl;
    
    // Connect to localhost (server must be running separately)
    hosting_ = connectToServer("127.0.0.1", port, playerName_);
    return hosting_;
}

void NetworkManager::stopServer() {
    disconnect();
}

// === Lobby Functions ===

void NetworkManager::requestRoomList() {
    if (!connected_ || !client_) {
        std::cerr << "[NetworkManager] Not connected to server" << std::endl;
        return;
    }

    // Send LOBBY_LIST_REQUEST packet (0x22 = ROOM_LIST)
    std::cout << "[NetworkManager] Sending ROOM_LIST request (0x22)" << std::endl;
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::LOBBY_LIST_REQUEST));
    client_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Requested room list" << std::endl;
}

void NetworkManager::createRoom(const std::string& roomName, uint8_t maxPlayers) {
    if (!connected_ || !client_) {
        std::cerr << "[NetworkManager] Not connected to server" << std::endl;
        return;
    }

    // Store locally for lobby display
    currentRoomName_ = roomName;
    currentMaxPlayers_ = maxPlayers;

    // Send ROOM_CREATE packet (0x20 = CREATE_ROOM)
    std::cout << "[NetworkManager] Sending CREATE_ROOM (0x20): " << roomName << ", max " << (int)maxPlayers << std::endl;
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::ROOM_CREATE));
    
    // Use Network::Serializer for proper format
    Network::Serializer serializer;
    serializer.writeString(roomName);
    serializer.write(maxPlayers);
    
    packet.payload = serializer.getBuffer();
    std::cout << "[NetworkManager] Packet payload size: " << packet.payload.size() << " bytes" << std::endl;
    client_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Creating room: " << roomName << " (max " << (int)maxPlayers << " players)" << std::endl;
}

void NetworkManager::joinRoom(uint32_t roomId) {
    if (!connected_ || !client_) {
        std::cerr << "[NetworkManager] Not connected to server" << std::endl;
        return;
    }

    // Send ROOM_JOIN packet (0x21 = JOIN_ROOM)
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::ROOM_JOIN));
    
    // Use Network::Serializer for proper format
    Network::Serializer serializer;
    serializer.write(roomId);
    
    packet.payload = serializer.getBuffer();
    client_->sendPacket(packet);
    
    // Set room ID immediately (will be confirmed by ROOM_JOINED response)
    currentRoomId_ = roomId;
    
    std::cout << "[NetworkManager] Joining room " << roomId << std::endl;
}

void NetworkManager::leaveRoom() {
    if (!connected_ || !client_) {
        std::cerr << "[NetworkManager] Not connected to server" << std::endl;
        return;
    }

    // If we're not in a room (no confirmation received), just reset state locally
    if (currentRoomId_ == 0) {
        std::cout << "[NetworkManager] Not in a confirmed room, resetting local state only" << std::endl;
        hosting_ = false;
        roomPlayers_.clear();
        return;
    }

    // Send ROOM_LEAVE packet
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::ROOM_LEAVE));
    
    // Payload: roomId (4 bytes)
    packet.payload.resize(sizeof(uint32_t));
    std::memcpy(packet.payload.data(), &currentRoomId_, sizeof(uint32_t));
    
    client_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Leaving room " << currentRoomId_ << std::endl;
    
    // Reset local state
    currentRoomId_ = 0;
    hosting_ = false;
    roomPlayers_.clear();
    currentRoomName_.clear();
    currentMaxPlayers_ = 4;
    chatMessages_.clear();
    chatVersion_++;
}

void NetworkManager::setReady(bool ready) {
    if (!connected_ || !client_) {
        std::cerr << "[NetworkManager] Not connected to server" << std::endl;
        return;
    }

    if (currentRoomId_ == 0) {
        std::cerr << "[NetworkManager] Not in a room" << std::endl;
        return;
    }

    // Send PLAYER_READY packet
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::PLAYER_READY));
    
    // Payload: ready (1 byte)
    packet.payload.resize(1);
    packet.payload[0] = ready ? 1 : 0;
    
    client_->sendPacket(packet);
    
    std::cout << "[NetworkManager] Set ready: " << (ready ? "true" : "false") << std::endl;
}

void NetworkManager::startGame() {
    if (!connected_ || !client_) {
        std::cerr << "[NetworkManager] Not connected to server" << std::endl;
        return;
    }

    if (currentRoomId_ == 0) {
        std::cerr << "[NetworkManager] Not in a room" << std::endl;
        return;
    }

    if (!hosting_) {
        std::cerr << "[NetworkManager] Only host can start game" << std::endl;
        return;
    }

    // Send GAME_START packet (0x23)
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::GAME_START));
    
    // Payload: roomId (4 bytes)
    Network::Serializer serializer;
    serializer.write(currentRoomId_);
    packet.payload = serializer.getBuffer();
    
    client_->sendPacket(packet);
    
    std::cout << "[NetworkManager] 🚀 Sending GAME_START for room " << currentRoomId_ << std::endl;
}

void NetworkManager::sendInput(bool up, bool down, bool left, bool right, bool fire, uint8_t chargeLevel) {
    if (!connected_ || !client_ || !inGame_) {
        return;  // Silently ignore if not in game
    }

    // Build input packet
    RType::ClientInput input;
    input.playerId = static_cast<uint8_t>(clientId_);
    input.inputMask = RType::ClientInput::buildInputMask(up, down, left, right, fire);
    input.chargeLevel = chargeLevel;

    // Create and send packet
    NetworkPacket packet = RType::Protocol::createInputPacket(input);
    client_->sendPacket(packet);
}

void NetworkManager::sendChatMessage(const std::string& message) {
    if (!connected_ || !client_ || currentRoomId_ == 0) {
        std::cerr << "[NetworkManager] Cannot send chat: not connected or not in a room" << std::endl;
        return;
    }

    if (message.empty()) return;

    // Build chat packet: senderId (uint32), senderName (string), message (string), roomId (uint32)
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::CHAT_MESSAGE));
    Network::Serializer serializer;
    serializer.write(clientId_);
    serializer.writeString(playerName_);
    serializer.writeString(message);
    serializer.write(currentRoomId_);
    packet.payload = serializer.getBuffer();

    client_->sendPacket(packet);
    std::cout << "[NetworkManager] 💬 Chat sent: " << message << std::endl;
}

void NetworkManager::update() {
    if (!client_ || !connected_) {
        return;
    }

    // Update client (handles keep-alive pings)
    client_->update(0.0f);

    // Process incoming packets
    client_->process();
    processIncomingPackets();
}

void NetworkManager::processIncomingPackets() {
    while (client_->hasReceivedPackets()) {
        NetworkPacket packet = client_->getNextReceivedPacket();
        
        // Serialize packet to get raw data for handlePacket
        auto serialized = packet.serialize();
        handlePacket(serialized.data(), serialized.size());
    }
}

void NetworkManager::handlePacket(const char* data, size_t length) {
    if (length < sizeof(PacketHeader)) {
        return;
    }

    // Read packet type from header
    PacketHeader header = PacketHeader::deserialize(data);
    auto type = static_cast<Network::PacketType>(header.type);

    // Payload starts after header
    const char* payload = data + sizeof(PacketHeader);
    size_t payloadSize = length - sizeof(PacketHeader);

    switch (type) {
        case Network::PacketType::SERVER_ACCEPT: {
            // Extract player ID from payload
            if (payloadSize >= 1) {
                clientId_ = static_cast<uint32_t>(static_cast<uint8_t>(payload[0]));
                std::cout << "[NetworkManager] Server accepted connection, assigned Player ID: " << clientId_ << std::endl;
            } else {
                std::cout << "[NetworkManager] Server accepted connection (no ID in payload)" << std::endl;
            }
            break;
        }

        case Network::PacketType::PING: {
            // Server sent PING (CLIENT_PING = 0x03), respond with PONG
            NetworkPacket pong(static_cast<uint16_t>(Network::PacketType::PONG));
            client_->sendPacket(pong);
            // Don't log to avoid spam
            break;
        }

        case Network::PacketType::PONG: {
            // Server ping reply (SERVER_PING_REPLY = 0x15)
            // Don't log to avoid spam
            break;
        }

        case Network::PacketType::LOBBY_LIST_RESPONSE: {
            // Parse room list from payload using Network::Deserializer
            std::vector<Network::RoomInfo> rooms;
            
            try {
                Network::Deserializer deserializer(payload, payloadSize);
                uint32_t count = deserializer.read<uint32_t>();
                
                for (uint32_t i = 0; i < count; ++i) {
                    Network::RoomInfo room;
                    room.roomId = deserializer.read<uint32_t>();
                    
                    // Read room name (string format: uint32_t length + chars)
                    std::string name = deserializer.readString();
                    std::strncpy(room.roomName, name.c_str(), sizeof(room.roomName) - 1);
                    room.roomName[sizeof(room.roomName) - 1] = '\0';
                    
                    room.currentPlayers = deserializer.read<uint8_t>();
                    room.maxPlayers = deserializer.read<uint8_t>();
                    room.inGame = deserializer.read<uint8_t>() != 0;
                    
                    rooms.push_back(room);
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[NetworkManager] Error parsing room list: " << e.what() << std::endl;
                break;
            }

            roomList_ = rooms;
            roomListVersion_++;
            std::cout << "[NetworkManager] Received " << rooms.size() << " rooms (version " << roomListVersion_ << ")" << std::endl;
            for (const auto& r : rooms) {
                std::cout << "[NetworkManager]   Room '" << r.roomName << "' (" 
                          << (int)r.currentPlayers << "/" << (int)r.maxPlayers 
                          << ") inGame=" << r.inGame << std::endl;
            }
            
            if (onRoomList_) {
                onRoomList_(rooms);
            }
            break;
        }

        case Network::PacketType::ROOM_CREATE: {
            // This is the request packet type we send, not what we receive
            std::cout << "[NetworkManager] Warning: Received ROOM_CREATE request packet (should only send these)" << std::endl;
            break;
        }

        case Network::PacketType::ROOM_CREATED: {
            // Server confirmation of room creation (ROOM_CREATED = 0x32)
            if (payloadSize >= sizeof(uint32_t)) {
                Network::Deserializer deserializer(payload, payloadSize);
                uint32_t roomId = deserializer.read<uint32_t>();
                currentRoomId_ = roomId;
                hosting_ = true;

                std::cout << "[NetworkManager] ✅ Room created with ID " << roomId << std::endl;
            }
            break;
        }

        case Network::PacketType::ROOM_JOIN: {
            // This is the request packet type we send, not what we receive
            std::cout << "[NetworkManager] Warning: Received ROOM_JOIN request packet (should only send these)" << std::endl;
            break;
        }

        case Network::PacketType::ROOM_JOINED: {
            // Server confirmation of room join (ROOM_JOINED = 0x30)
            // Format: roomId(uint32) + roomName(string) + maxPlayers(uint8) + hostPlayerId(uint32)
            try {
                Network::Deserializer deserializer(payload, payloadSize);
                uint32_t roomId = deserializer.read<uint32_t>();
                std::string roomName = deserializer.readString();
                uint8_t maxPlayers = deserializer.read<uint8_t>();
                uint32_t hostPlayerId = deserializer.read<uint32_t>();
                
                currentRoomId_ = roomId;
                currentRoomName_ = roomName;
                currentMaxPlayers_ = maxPlayers;

                std::cout << "[NetworkManager] ✅ Joined room " << roomId << " (" << roomName 
                          << ", " << (int)maxPlayers << " max players, host: " << hostPlayerId << ")" << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "[NetworkManager] Error parsing ROOM_JOINED: " << e.what() << std::endl;
            }
            break;
        }

        case Network::PacketType::ROOM_UPDATE: {
            // Room state update (ROOM_PLAYERS_UPDATE - player list, ready states, etc.)
            try {
                Network::Deserializer deserializer(payload, payloadSize);
                uint32_t roomId = deserializer.read<uint32_t>();
                uint32_t playerCount = deserializer.read<uint32_t>();
                
                roomPlayers_.clear();
                
                for (uint32_t i = 0; i < playerCount; ++i) {
                    PlayerInfo player;
                    player.playerId = deserializer.read<uint32_t>();
                    player.playerName = deserializer.readString();
                    player.isHost = deserializer.read<bool>();
                    player.isReady = deserializer.read<bool>();
                    roomPlayers_.push_back(player);
                }
                
                std::cout << "[NetworkManager] Room " << roomId << " update: " << playerCount << " players" << std::endl;
                for (const auto& player : roomPlayers_) {
                    std::cout << "  - " << player.playerName << " (ID:" << player.playerId 
                              << ", Host:" << (player.isHost ? "Yes" : "No")
                              << ", Ready:" << (player.isReady ? "Yes" : "No") << ")" << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[NetworkManager] Error parsing ROOM_UPDATE: " << e.what() << std::endl;
            }
            break;
        }

        case Network::PacketType::ROOM_LEAVE: {
            // Confirmation of leaving room
            currentRoomId_ = 0;
            currentRoomName_.clear();
            currentMaxPlayers_ = 4;
            hosting_ = false;
            inGame_ = false;

            std::cout << "[NetworkManager] Left room" << std::endl;
            break;
        }

        case Network::PacketType::GAME_START: {
            std::cout << "[NetworkManager] 🎮 Game starting!" << std::endl;
            inGame_ = true;  // Mark as in game
            if (gameStartCallback_) {
                gameStartCallback_();
            }
            break;
        }

        case Network::PacketType::ENTITY_UPDATE: {
            // WORLD_SNAPSHOT (0x11) - Game state updates
            if (!inGame_) {
                break;  // Ignore during lobby
            }
            
            try {
                // Parse the full packet (header + payload were passed as data)
                NetworkPacket fullPacket = NetworkPacket::deserialize(data, length);
                
                // Parse world snapshot
                auto [snapHeader, entities] = RType::Protocol::parseWorldSnapshot(fullPacket);
                
                // Call snapshot callback if set
                if (onWorldSnapshot_) {
                    onWorldSnapshot_(entities);
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[NetworkManager] Error parsing WORLD_SNAPSHOT: " << e.what() << std::endl;
            }
            break;
        }

        case Network::PacketType::LEVEL_CHANGE: {
            if (length >= sizeof(PacketHeader) + 1) {
                // Extract level ID from payload (first byte after header)
                const char* payloadStart = data + sizeof(PacketHeader);
                uint8_t newLevel = static_cast<uint8_t>(*payloadStart);
                std::cout << "[NetworkManager] 🎮 LEVEL_CHANGE received: Level " << (int)newLevel << std::endl;
                if (onLevelChange_) {
                    onLevelChange_(newLevel);
                }
            }
            break;
        }

        case Network::PacketType::GAME_OVER: {
            uint32_t totalScore = 0;
            if (payloadSize >= sizeof(uint32_t)) {
                std::memcpy(&totalScore, payload, sizeof(uint32_t));
            }
            std::cout << "[NetworkManager] 💀 GAME_OVER received! Score: " << totalScore << std::endl;
            if (onGameOver_) {
                onGameOver_(totalScore);
            }
            break;
        }

        case Network::PacketType::GAME_VICTORY: {
            uint32_t totalScore = 0;
            if (payloadSize >= sizeof(uint32_t)) {
                std::memcpy(&totalScore, payload, sizeof(uint32_t));
            }
            std::cout << "[NetworkManager] 🏆 GAME_VICTORY received! Score: " << totalScore << std::endl;
            if (onVictory_) {
                onVictory_(totalScore);
            }
            break;
        }

        case Network::PacketType::CHAT_MESSAGE: {
            try {
                Network::Deserializer deserializer(payload, payloadSize);
                uint32_t senderId = deserializer.read<uint32_t>();
                std::string senderName = deserializer.readString();
                std::string message = deserializer.readString();
                uint32_t roomId = deserializer.read<uint32_t>();

                ChatMessage chatMsg;
                chatMsg.senderName = senderName;
                chatMsg.message = message;
                chatMessages_.push_back(chatMsg);
                chatVersion_++;

                // Keep only the last 50 messages
                if (chatMessages_.size() > 50) {
                    chatMessages_.erase(chatMessages_.begin());
                }

                std::cout << "[NetworkManager] 💬 [" << senderName << "]: " << message << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[NetworkManager] Error parsing CHAT_MESSAGE: " << e.what() << std::endl;
            }
            break;
        }

        default:
            // Don't spam log for unknown packets
            break;
    }
}
