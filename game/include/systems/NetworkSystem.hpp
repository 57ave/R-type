#pragma once

#include "ecs/System.hpp"
#include "ecs/Coordinator.hpp"
#include "network/NetworkClient.hpp"
#include "network/RTypeProtocol.hpp"
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
    
    NetworkSystem(ECS::Coordinator* coordinator, std::shared_ptr<NetworkClient> client)
        : coordinator_(coordinator), networkClient_(client), localPlayerId_(0) {}
    
    void setEntityCreatedCallback(EntityCallback callback) {
        entityCreatedCallback_ = callback;
    }
    
    void setEntityDestroyedCallback(EntityDestroyCallback callback) {
        entityDestroyedCallback_ = callback;
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
            default:
                std::cout << "[NetworkSystem] Unknown packet type: " << packet.header.type << std::endl;
                break;
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
        
        // Add NetworkId component
        bool isLocal = (state.type == EntityType::ENTITY_PLAYER && 
                       state.id == localPlayerId_);
        coordinator_->AddComponent(entity, NetworkId(state.id, isLocal, localPlayerId_, state.playerLine));
        
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
                    enemyTag.enemyType = "basic"; // TODO: Map from state.enemyType
                    coordinator_->AddComponent(entity, enemyTag);
                }
                std::cout << "[NetworkSystem] Created Enemy entity " << entity << " at (" << state.x << ", " << state.y << ")" << std::endl;
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

public:
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

private:
    ECS::Coordinator* coordinator_;
    std::shared_ptr<NetworkClient> networkClient_;
    std::unordered_map<uint32_t, ECS::Entity> networkIdToEntity_;
    uint8_t localPlayerId_;
    EntityCallback entityCreatedCallback_;
    EntityDestroyCallback entityDestroyedCallback_;
};

} // namespace systems
} // namespace engine
} // namespace eng
