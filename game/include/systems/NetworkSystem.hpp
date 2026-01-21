#pragma once

#include "ecs/System.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Types.hpp"
#include "network/NetworkClient.hpp"
#include "network/RTypeProtocol.hpp"
#include "network/NetworkBindings.hpp"
#include "GameStateManager.hpp"
#include <components/NetworkId.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Health.hpp>
#include <components/Tag.hpp>
#include <components/ShootEmUpTags.hpp>
#include <memory>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <cstring>

namespace eng {
namespace engine {
namespace systems {

class NetworkSystem : public ECS::System {
public:
    using EntityCallback = std::function<void(ECS::Entity)>;
    using EntityDestroyCallback = std::function<void(ECS::Entity, uint32_t)>; // entity, networkId
    using GameStartCallback = std::function<void()>; // Called when GAME_START is received
    
    NetworkSystem(ECS::Coordinator* coordinator, std::shared_ptr<NetworkClient> client)
        : coordinator_(coordinator), networkClient_(client), localPlayerId_(0) {}
    
    void setEntityCreatedCallback(EntityCallback callback) {
        entityCreatedCallback_ = callback;
    }
    
    void setEntityDestroyedCallback(EntityDestroyCallback callback) {
        entityDestroyedCallback_ = callback;
    }
    
    void setGameStartCallback(GameStartCallback callback) {
        gameStartCallback_ = callback;
    }

    void Init() override {
        std::cout << "[NetworkSystem] Initialized" << std::endl;
    }

    void Update(float dt) override {
        if (!networkClient_ || !networkClient_->isConnected()) {
            return;
        }

        // Process network packets
        networkClient_->process();

        // Handle received packets
        while (networkClient_->hasReceivedPackets()) {
            NetworkPacket packet = networkClient_->getNextReceivedPacket();
            handlePacket(packet);
        }

        // Send input for local player (if exists)
        sendLocalPlayerInput(dt);
    }

    void Shutdown() override {
        if (networkClient_) {
            networkClient_->disconnect();
        }
        std::cout << "[NetworkSystem] Shutdown" << std::endl;
    }

    void setLocalPlayerId(uint8_t playerId) {
        localPlayerId_ = playerId;
    }

    uint8_t getLocalPlayerId() const {
        return localPlayerId_;
    }

    // Called by game code to send input
    void sendInput(uint8_t inputMask, uint8_t chargeLevel = 0) {
        if (networkClient_ && networkClient_->isConnected()) {
            // Build RType protocol packet
            NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CLIENT_INPUT));
            ClientInput input;
            input.playerId = localPlayerId_;
            input.inputMask = inputMask;
            input.chargeLevel = chargeLevel;
            
            // Serialize input to payload
            std::vector<char> payload(sizeof(ClientInput));
            std::memcpy(payload.data(), &input, sizeof(ClientInput));
            packet.setPayload(payload);
            
            networkClient_->sendPacket(packet);
        }
    }

    // Request server to toggle pause for the room (server will validate host)
    void sendTogglePause() {
        if (!networkClient_ || !networkClient_->isConnected()) return;
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::CLIENT_TOGGLE_PAUSE));
        packet.header.timestamp = 0;
        networkClient_->sendPacket(packet);
        std::cout << "[NetworkSystem] Sent CLIENT_TOGGLE_PAUSE request to server" << std::endl;
    }

private:
    void handlePacket(const NetworkPacket& packet) {
        auto type = static_cast<GamePacketType>(packet.header.type);

        switch (type) {
            case GamePacketType::SERVER_WELCOME:
                handleServerWelcome(packet);
                break;
            case GamePacketType::WORLD_SNAPSHOT:
                handleWorldSnapshot(packet);
                break;
            case GamePacketType::ENTITY_SPAWN:
                handleEntitySpawn(packet);
                break;
            case GamePacketType::ENTITY_DESTROY:
                handleEntityDestroy(packet);
                break;
            case GamePacketType::PLAYER_DIED:
                handlePlayerDied(packet);
                break;
            case GamePacketType::CLIENT_LEFT:
                handleClientLeft(packet);
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
            case GamePacketType::SERVER_SET_PAUSE:
                handleServerSetPause(packet);
                break;
            case GamePacketType::CHAT_MESSAGE:
                handleChatMessage(packet);
                break;
            default:
                std::cout << "[NetworkSystem] Unknown packet type: " << packet.header.type << std::endl;
                break;
        }
    }


    void handleServerSetPause(const NetworkPacket& packet) {
        if (packet.payload.size() < 1) return;
        uint8_t paused = static_cast<uint8_t>(packet.payload[0]);
        if (paused) {
            std::cout << "[NetworkSystem] Server requested PAUSE" << std::endl;
            GameStateManager::Instance().SetState(GameState::Paused);
        } else {
            std::cout << "[NetworkSystem] Server requested RESUME" << std::endl;
            GameStateManager::Instance().SetState(GameState::Playing);
        }
    }

    void handleServerWelcome(const NetworkPacket& packet) {
        if (packet.payload.size() >= 1) {
            localPlayerId_ = static_cast<uint8_t>(packet.payload[0]);
            networkClient_->setPlayerId(localPlayerId_);
            std::cout << "[NetworkSystem] Received SERVER_WELCOME. Player ID: " << (int)localPlayerId_ << std::endl;
        }
    }

    void handleWorldSnapshot(const NetworkPacket& packet) {
        if (packet.payload.size() < sizeof(SnapshotHeader)) {
            return;
        }

        SnapshotHeader header = SnapshotHeader::deserialize(packet.payload.data());
        size_t offset = sizeof(SnapshotHeader);

        // Update or create entities based on snapshot
        for (uint32_t i = 0; i < header.entityCount; ++i) {
            if (offset + sizeof(EntityState) > packet.payload.size()) {
                break;
            }

            EntityState state = EntityState::deserialize(packet.payload.data() + offset);
            offset += sizeof(EntityState);

            updateOrCreateEntity(state);
        }
    }

    void handleEntitySpawn(const NetworkPacket& packet) {
        if (packet.payload.size() >= sizeof(EntityState)) {
            EntityState state = EntityState::deserialize(packet.payload.data());
            createEntityFromState(state);
        }
    }

    void handleEntityDestroy(const NetworkPacket& packet) {
        if (packet.payload.size() >= sizeof(uint32_t)) {
            uint32_t networkId;
            std::memcpy(&networkId, packet.payload.data(), sizeof(uint32_t));
            
            auto it = networkIdToEntity_.find(networkId);
            if (it != networkIdToEntity_.end()) {
                ECS::Entity entity = it->second;
                
                // Notify callback before destroying (for explosion effects)
                if (entityDestroyedCallback_) {
                    entityDestroyedCallback_(entity, networkId);
                }
                
                coordinator_->DestroyEntity(entity);
                networkIdToEntity_.erase(it);
            }
        }
    }

    void handlePlayerDied(const NetworkPacket&) {
        std::cout << "[NetworkSystem] Player died" << std::endl;
    }

    void handleClientLeft(const NetworkPacket&) {
        std::cout << "[NetworkSystem] Client left" << std::endl;
    }

    void handleRoomListReply(const NetworkPacket& packet) {
        std::cout << "[NetworkSystem] Received ROOM_LIST_REPLY" << std::endl;
        
        // Deserialize room list
        if (packet.payload.size() < sizeof(uint32_t)) {
            std::cout << "[NetworkSystem] Empty room list received" << std::endl;
            RType::Network::NetworkBindings::OnRoomListReceived({});
            return;
        }
        
        try {
            Network::Deserializer deserializer(packet.payload);
            uint32_t roomCount = deserializer.read<uint32_t>();
            
            std::vector<RoomInfo> rooms;
            for (uint32_t i = 0; i < roomCount; ++i) {
                RoomInfo room;
                room.id = deserializer.read<uint32_t>();
                room.name = deserializer.readString();
                room.currentPlayers = deserializer.read<uint8_t>();
                room.maxPlayers = deserializer.read<uint8_t>();
                rooms.push_back(room);
            }
            
            // Forward to NetworkBindings
            RType::Network::NetworkBindings::OnRoomListReceived(rooms);
        } catch (const std::exception& e) {
            std::cerr << "[NetworkSystem] Error deserializing ROOM_LIST_REPLY: " << e.what() << std::endl;
            RType::Network::NetworkBindings::OnRoomListReceived({});
        }
    }

    void handleRoomCreated(const NetworkPacket& packet) {
        std::cout << "[NetworkSystem] Received ROOM_CREATED" << std::endl;
        
        if (packet.payload.size() < sizeof(uint32_t)) {
            std::cerr << "[NetworkSystem] ROOM_CREATED payload too small" << std::endl;
            return;
        }
        
        try {
            Network::Deserializer deserializer(packet.payload);
            uint32_t roomId = deserializer.read<uint32_t>();
            
            std::cout << "[NetworkSystem] Room created with ID: " << roomId << std::endl;
            
            // Forward to NetworkBindings
            RType::Network::NetworkBindings::OnRoomCreated(roomId);
        } catch (const std::exception& e) {
            std::cerr << "[NetworkSystem] Error deserializing ROOM_CREATED: " << e.what() << std::endl;
        }
    }

    void handleRoomJoined(const NetworkPacket& packet) {
        std::cout << "[NetworkSystem] Received ROOM_JOINED" << std::endl;
        
        // Check payload size (roomId + roomName + maxPlayers + hostPlayerId)
        if (packet.payload.size() < sizeof(uint32_t) + 1) {
            std::cerr << "[NetworkSystem] ROOM_JOINED payload too small: " << packet.payload.size() << " bytes" << std::endl;
            return;
        }
        
        // Deserialize room info
        try {
            Network::Deserializer deserializer(packet.payload);
            uint32_t roomId = deserializer.read<uint32_t>();
            std::string roomName = deserializer.readString();
            uint8_t maxPlayers = deserializer.read<uint8_t>();
            uint32_t hostPlayerId = deserializer.read<uint32_t>();
            
            bool isHost = (hostPlayerId == localPlayerId_);
            
            std::cout << "[NetworkSystem] Joined room " << roomId << ": " << roomName 
                      << " (max: " << static_cast<int>(maxPlayers) << ", host: " 
                      << (isHost ? "YES" : "NO") << ")" << std::endl;
            
            // Forward to NetworkBindings with all info
            RType::Network::NetworkBindings::OnRoomJoined(roomId, roomName, maxPlayers, isHost);
        } catch (const std::exception& e) {
            std::cerr << "[NetworkSystem] Error deserializing ROOM_JOINED: " << e.what() << std::endl;
        }
    }

    void handleChatMessage(const NetworkPacket& packet) {
        // Deserialize chat message
        try {
            ChatMessagePayload payload = ChatMessagePayload::deserialize(packet.payload);
            
            // Forward to NetworkBindings
            RType::Network::NetworkBindings::OnChatMessage(payload.senderName, payload.message);
        } catch (const std::exception& e) {
            std::cerr << "[NetworkSystem] Error deserializing CHAT_MESSAGE: " << e.what() << std::endl;
        }
    }


    void handleGameStart(const NetworkPacket&) {
        std::cout << "[NetworkSystem] Received GAME_START - transitioning to Playing state" << std::endl;
        
        // Call game start callback if set
        if (gameStartCallback_) {
            gameStartCallback_();
        }
        
        // Notify Lua if callback exists
        RType::Network::NetworkBindings::OnGameStarting(0);
    }

    void updateOrCreateEntity(const EntityState& state) {
        auto it = networkIdToEntity_.find(state.id);
        
        if (it != networkIdToEntity_.end()) {
            // Update existing entity
            ECS::Entity entity = it->second;
            
            if (coordinator_->HasComponent<Position>(entity)) {
                auto& pos = coordinator_->GetComponent<Position>(entity);
                pos.x = state.x;
                pos.y = state.y;
            }
            
            if (coordinator_->HasComponent<Velocity>(entity)) {
                auto& vel = coordinator_->GetComponent<Velocity>(entity);
                vel.dx = state.vx;
                vel.dy = state.vy;
            }
            
            if (coordinator_->HasComponent<Health>(entity)) {
                auto& health = coordinator_->GetComponent<Health>(entity);
                health.current = state.hp;
            }
        } else {
            // Create new entity
            createEntityFromState(state);
        }
    }

    void createEntityFromState(const EntityState& state) {
        ECS::Entity entity = coordinator_->CreateEntity();
        
        // Add NetworkId component. Use state.playerId to determine ownership
        bool isLocal = (state.type == EntityType::ENTITY_PLAYER && 
                   state.playerId == localPlayerId_);
        coordinator_->AddComponent(entity, NetworkId(state.id, isLocal, state.playerId, state.playerLine));
        
        // Add Position
        coordinator_->AddComponent(entity, Position{static_cast<float>(state.x), static_cast<float>(state.y)});
        
        // Add Velocity
        coordinator_->AddComponent(entity, Velocity{static_cast<float>(state.vx), static_cast<float>(state.vy)});
        
        // Add Health
        coordinator_->AddComponent(entity, Health{state.hp, state.hp});
        
        // Add Tag based on entity type
        switch (state.type) {
            case EntityType::ENTITY_PLAYER:
                coordinator_->AddComponent(entity, Tag{"Player"});
                break;
            case EntityType::ENTITY_MONSTER:
                coordinator_->AddComponent(entity, Tag{"Enemy"});
                // Add EnemyTag with type from server
                {
                    ShootEmUp::Components::EnemyTag enemyTag;
                    // Map numeric enemy type to string name
                    switch (state.enemyType) {
                        case 0: enemyTag.enemyType = "basic"; break;
                        case 1: enemyTag.enemyType = "zigzag"; break;
                        case 2: enemyTag.enemyType = "sine"; break;
                        case 3: enemyTag.enemyType = "kamikaze"; break;
                        case 4: enemyTag.enemyType = "turret"; break;
                        case 5: enemyTag.enemyType = "boss"; break;
                        default: enemyTag.enemyType = "basic"; break;
                    }
                    coordinator_->AddComponent(entity, enemyTag);
                }
                std::cout << "[NetworkSystem] Created Enemy entity " << entity 
                          << " (type: " << (int)state.enemyType << ") at (" 
                          << state.x << ", " << state.y << ")" << std::endl;
                break;
            case EntityType::ENTITY_PLAYER_MISSILE:
                coordinator_->AddComponent(entity, Tag{"PlayerBullet"});
                // Add ProjectileTag with charge level and type
                {
                    ShootEmUp::Components::ProjectileTag projTag;
                    projTag.projectileType = (state.chargeLevel > 0) ? "charged" : "normal";
                    projTag.chargeLevel = state.chargeLevel;
                    coordinator_->AddComponent(entity, projTag);
                }
                std::cout << "[NetworkSystem] Created PlayerBullet entity " << entity << " at (" << state.x << ", " << state.y << ")" << std::endl;
                break;
            case EntityType::ENTITY_MONSTER_MISSILE:
                coordinator_->AddComponent(entity, Tag{"EnemyBullet"});
                {
                    ShootEmUp::Components::ProjectileTag projTag;
                    projTag.projectileType = "normal";
                    projTag.chargeLevel = 0;
                    coordinator_->AddComponent(entity, projTag);
                }
                break;
            case EntityType::ENTITY_EXPLOSION:
                coordinator_->AddComponent(entity, Tag{"Explosion"});
                break;
            default:
                break;
        }
        
        // Store mapping
        networkIdToEntity_[state.id] = entity;
        
        // Notify callback
        if (entityCreatedCallback_) {
            entityCreatedCallback_(entity);
        }
        
        std::cout << "[NetworkSystem] Created entity " << entity << " for network ID " << state.id << std::endl;
    }

    void sendLocalPlayerInput(float) {
        // This will be called by the game to send input
        // The actual input gathering happens in the game code
    }

private:
    ECS::Coordinator* coordinator_;
    std::shared_ptr<NetworkClient> networkClient_;
    std::unordered_map<uint32_t, ECS::Entity> networkIdToEntity_;
    uint8_t localPlayerId_;
    EntityCallback entityCreatedCallback_;
    EntityDestroyCallback entityDestroyedCallback_;
    GameStartCallback gameStartCallback_;
};

} // namespace systems
} // namespace engine
} // namespace eng
