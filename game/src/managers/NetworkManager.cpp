/**
 * NetworkManager.cpp - Network Manager Implementation (Real ASIO)
 * Phase 5.3: Real network using engine's NetworkClient
 * Phase 6.5: Gameplay synchronization (WORLD_SNAPSHOT + CLIENT_INPUT)
 */

#include "managers/NetworkManager.hpp"
#include "network/Serializer.hpp"
#include "network/RTypeProtocol.hpp"
#include <scripting/LuaState.hpp>
#include <sol/sol.hpp>
#include "core/Logger.hpp"
#include <sstream>
#include <cstring>
#include <thread>
#include <chrono>

NetworkManager::~NetworkManager() {
    shutdown();
}

bool NetworkManager::initialize() {
    LOG_INFO("NetworkManager", "Initialized");
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
        LOG_INFO("NetworkManager", "Disconnecting existing connection...");
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
        LOG_INFO("NetworkManager", (std::ostringstream{} << "Sending CLIENT_HELLO to " << address << ":" << port).str());
        client_->sendHello();
        
        connected_ = true;
        LOG_INFO("NetworkManager", (std::ostringstream{} << "Connected to " << address << ":" << port << " as " << playerName).str());
        
        if (onConnection_) {
            onConnection_(true, "Connected successfully");
        }
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("NetworkManager", (std::ostringstream{} << "Connection error: " << e.what()).str());
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
    LOG_INFO("NetworkManager", "disconnect() called!");
    
    if (client_ && connected_) {
        // NetworkClient::disconnect() already sends CLIENT_DISCONNECT packet
        // So we just need to call it, not send our own packet
        client_->disconnect();
        LOG_INFO("NetworkManager", "Disconnected from server");
    }
    
    client_.reset();
    connected_ = false;
    hosting_ = false;
    currentRoomId_ = 0;
}

bool NetworkManager::startServer(uint16_t port, uint8_t maxPlayers) {
    LOG_INFO("NetworkManager", "Server should be started separately");

    // Read server_ip from game_config.lua (single source of truth)
    std::string connectIp = "127.0.0.1";
    try {
        auto& lua = Scripting::LuaState::Instance().GetState();
        lua.script_file("assets/scripts/config/game_config.lua");
        sol::table gameConfig = lua["Game"];
        if (gameConfig.valid()) {
            sol::optional<sol::table> netT = gameConfig["network"];
            if (netT) {
                connectIp = netT.value().get_or<std::string>("server_ip", connectIp);
            }
        }
    } catch (...) {
        LOG_WARNING("NetworkManager", "Could not read server_ip from game_config.lua, defaulting to 127.0.0.1");
    }

    LOG_INFO("NetworkManager", (std::ostringstream{} << "Connecting to " << connectIp << ":" << port).str());
    hosting_ = connectToServer(connectIp, port, playerName_);
    return hosting_;
}

void NetworkManager::stopServer() {
    disconnect();
}

// === Lobby Functions ===

void NetworkManager::requestRoomList() {
    if (!connected_ || !client_) {
        LOG_ERROR("NetworkManager", "Not connected to server");
        return;
    }

    // Send LOBBY_LIST_REQUEST packet (0x22 = ROOM_LIST)
    LOG_INFO("NetworkManager", "Sending ROOM_LIST request (0x22)");
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::LOBBY_LIST_REQUEST));
    client_->sendPacket(packet);
    
    LOG_INFO("NetworkManager", "Requested room list");
}

void NetworkManager::createRoom(const std::string& roomName, uint8_t maxPlayers) {
    if (!connected_ || !client_) {
        LOG_ERROR("NetworkManager", "Not connected to server");
        return;
    }

    // Store locally for lobby display
    currentRoomName_ = roomName;
    currentMaxPlayers_ = maxPlayers;

    // Send ROOM_CREATE packet (0x20 = CREATE_ROOM)
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Sending CREATE_ROOM (0x20): " << roomName << ", max " << (int)maxPlayers).str());
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::ROOM_CREATE));
    
    // Use Network::Serializer for proper format
    Network::Serializer serializer;
    serializer.writeString(roomName);
    serializer.write(maxPlayers);
    
    packet.payload = serializer.getBuffer();
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Packet payload size: " << packet.payload.size() << " bytes").str());
    client_->sendPacket(packet);
    
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Creating room: " << roomName << " (max " << (int)maxPlayers << " players)").str());
}

void NetworkManager::joinRoom(uint32_t roomId) {
    if (!connected_ || !client_) {
        LOG_ERROR("NetworkManager", "Not connected to server");
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
    
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Joining room " << roomId).str());
}

void NetworkManager::leaveRoom() {
    if (!connected_ || !client_) {
        LOG_ERROR("NetworkManager", "Not connected to server");
        return;
    }

    // If we're not in a room (no confirmation received), just reset state locally
    if (currentRoomId_ == 0) {
        LOG_INFO("NetworkManager", "Not in a confirmed room, resetting local state only");
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
    
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Leaving room " << currentRoomId_).str());
    
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
        LOG_ERROR("NetworkManager", "Not connected to server");
        return;
    }

    if (currentRoomId_ == 0) {
        LOG_ERROR("NetworkManager", "Not in a room");
        return;
    }

    // Send PLAYER_READY packet
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::PLAYER_READY));
    
    // Payload: ready (1 byte)
    packet.payload.resize(1);
    packet.payload[0] = ready ? 1 : 0;
    
    client_->sendPacket(packet);
    
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Set ready: " << (ready ? "true" : "false")).str());
}

void NetworkManager::startGame() {
    if (!connected_ || !client_) {
        LOG_ERROR("NetworkManager", "Not connected to server");
        return;
    }

    if (currentRoomId_ == 0) {
        LOG_ERROR("NetworkManager", "Not in a room");
        return;
    }

    if (!hosting_) {
        LOG_ERROR("NetworkManager", "Only host can start game");
        return;
    }

    // Send GAME_START packet (0x23)
    NetworkPacket packet(static_cast<uint16_t>(Network::PacketType::GAME_START));
    
    // Payload: roomId (4 bytes)
    Network::Serializer serializer;
    serializer.write(currentRoomId_);
    packet.payload = serializer.getBuffer();
    
    client_->sendPacket(packet);
    
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Sending GAME_START for room " << currentRoomId_).str());
}

void NetworkManager::sendInput(bool up, bool down, bool left, bool right, bool fire, uint8_t chargeLevel) {
    if (!connected_ || !client_ || !inGame_) {
        return;  // Silently ignore if not in game
    }

    // Build input packet with monotonic sequence number
    RType::ClientInput input;
    input.playerId = static_cast<uint8_t>(clientId_);
    input.inputMask = RType::ClientInput::buildInputMask(up, down, left, right, fire);
    input.chargeLevel = chargeLevel;
    input.inputSeq = ++inputSequence_;

    // Create and send packet
    NetworkPacket packet = RType::Protocol::createInputPacket(input);
    client_->sendPacket(packet);
}

void NetworkManager::sendChatMessage(const std::string& message) {
    if (!connected_ || !client_ || currentRoomId_ == 0) {
        LOG_ERROR("NetworkManager", "Cannot send chat: not connected or not in a room");
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
    LOG_INFO("NetworkManager", (std::ostringstream{} << "Chat sent: " << message).str());
}

void NetworkManager::update(float deltaTime) {
    if (!client_ || !connected_) {
        return;
    }

    // Update client (handles keep-alive pings)
    client_->update(0.0f);

    // Periodic ping for RTT measurement (every 1 second)
    if (inGame_) {
        pingTimer_ += deltaTime;
        if (pingTimer_ >= 1.0f) {
            pingTimer_ = 0.0f;
            sendPing();
        }
    }

    // Process incoming packets
    client_->process();
    processIncomingPackets();
}

void NetworkManager::sendPing() {
    if (!client_ || !connected_) return;

    auto now = std::chrono::steady_clock::now();
    lastPingTimestamp_ = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

    NetworkPacket packet(static_cast<uint16_t>(RType::GamePacketType::CLIENT_PING));
    packet.header.timestamp = lastPingTimestamp_;
    client_->sendPacket(packet);
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
                LOG_INFO("NetworkManager", (std::ostringstream{} << "Server accepted connection, assigned Player ID: " << clientId_).str());
            } else {
                LOG_INFO("NetworkManager", "Server accepted connection (no ID in payload)");
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
            // Payload contains our original timestamp (4 bytes)
            if (payloadSize >= sizeof(uint32_t)) {
                uint32_t echoedTimestamp = 0;
                std::memcpy(&echoedTimestamp, payload, sizeof(uint32_t));
                if (echoedTimestamp == lastPingTimestamp_ && lastPingTimestamp_ != 0) {
                    auto now = std::chrono::steady_clock::now();
                    uint32_t nowMs = static_cast<uint32_t>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
                    rtt_ = static_cast<float>(nowMs - echoedTimestamp) / 1000.0f;
                    // Exponential moving average (alpha = 0.2)
                    if (smoothedRtt_ <= 0.0f) {
                        smoothedRtt_ = rtt_;
                    } else {
                        smoothedRtt_ = 0.8f * smoothedRtt_ + 0.2f * rtt_;
                    }
                }
            }
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
                LOG_ERROR("NetworkManager", (std::ostringstream{} << "Error parsing room list: " << e.what()).str());
                break;
            }

            roomList_ = rooms;
            roomListVersion_++;
            LOG_INFO("NetworkManager", (std::ostringstream{} << "Received " << rooms.size() << " rooms (version " << roomListVersion_ << ")").str());
            for (const auto& r : rooms) {
                LOG_INFO("NetworkManager", (std::ostringstream{} << "Room '" << r.roomName << "' (" << (int)r.currentPlayers << "/" << (int)r.maxPlayers << ") inGame=" << r.inGame).str());
            }
            
            if (onRoomList_) {
                onRoomList_(rooms);
            }
            break;
        }

        case Network::PacketType::ROOM_CREATE: {
            // This is the request packet type we send, not what we receive
            LOG_INFO("NetworkManager", "Warning: Received ROOM_CREATE request packet (should only send these)");
            break;
        }

        case Network::PacketType::ROOM_CREATED: {
            // Server confirmation of room creation (ROOM_CREATED = 0x32)
            if (payloadSize >= sizeof(uint32_t)) {
                Network::Deserializer deserializer(payload, payloadSize);
                uint32_t roomId = deserializer.read<uint32_t>();
                currentRoomId_ = roomId;
                hosting_ = true;

                LOG_INFO("NetworkManager", (std::ostringstream{} << "Room created with ID " << roomId).str());
            }
            break;
        }

        case Network::PacketType::ROOM_JOIN: {
            // This is the request packet type we send, not what we receive
            LOG_INFO("NetworkManager", "Warning: Received ROOM_JOIN request packet (should only send these)");
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

                LOG_INFO("NetworkManager", (std::ostringstream{} << "Joined room " << roomId << " (" << roomName << ", " << (int)maxPlayers << " max players, host: " << hostPlayerId << ")").str());
            }
            catch (const std::exception& e) {
                LOG_ERROR("NetworkManager", (std::ostringstream{} << "Error parsing ROOM_JOINED: " << e.what()).str());
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
                
                LOG_INFO("NetworkManager", (std::ostringstream{} << "Room " << roomId << " update: " << playerCount << " players").str());
                for (const auto& player : roomPlayers_) {
                    LOG_INFO("NetworkManager", (std::ostringstream{} << "  - " << player.playerName << " (ID:" << player.playerId << ", Host:" << (player.isHost ? "Yes" : "No") << ", Ready:" << (player.isReady ? "Yes" : "No") << ")").str());
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("NetworkManager", (std::ostringstream{} << "Error parsing ROOM_UPDATE: " << e.what()).str());
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

            LOG_INFO("NetworkManager", "Left room");
            break;
        }

        case Network::PacketType::GAME_START: {
            LOG_INFO("NetworkManager", "Game starting!");
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

                // Parse world snapshot (new format with acks)
                RType::WorldSnapshotData snapshotData = RType::Protocol::parseWorldSnapshot(fullPacket);

                // Reject out-of-order snapshots
                if (snapshotData.header.snapshotSeq <= lastSnapshotSeq_ && lastSnapshotSeq_ > 0) {
                    break;
                }
                lastSnapshotSeq_ = snapshotData.header.snapshotSeq;

                // Call snapshot callback with full data (entities + acks)
                if (onWorldSnapshot_) {
                    onWorldSnapshot_(snapshotData);
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("NetworkManager", (std::ostringstream{} << "Error parsing WORLD_SNAPSHOT: " << e.what()).str());
            }
            break;
        }

        case Network::PacketType::LEVEL_CHANGE: {
            if (length >= sizeof(PacketHeader) + 1) {
                // Extract level ID from payload (first byte after header)
                const char* payloadStart = data + sizeof(PacketHeader);
                uint8_t newLevel = static_cast<uint8_t>(*payloadStart);
                LOG_INFO("NetworkManager", (std::ostringstream{} << "LEVEL_CHANGE received: Level " << (int)newLevel).str());
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
            LOG_INFO("NetworkManager", (std::ostringstream{} << "GAME_OVER received! Score: " << totalScore).str());
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
            LOG_INFO("NetworkManager", (std::ostringstream{} << "GAME_VICTORY received! Score: " << totalScore).str());
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

                LOG_INFO("NetworkManager", (std::ostringstream{} << "[" << senderName << "]: " << message).str());
            } catch (const std::exception& e) {
                LOG_ERROR("NetworkManager", (std::ostringstream{} << "Error parsing CHAT_MESSAGE: " << e.what()).str());
            }
            break;
        }

        default:
            // Don't spam log for unknown packets
            break;
    }
}
