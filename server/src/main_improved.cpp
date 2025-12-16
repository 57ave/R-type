#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <random>
#include "network/NetworkServer.hpp"
#include "network/RTypeProtocol.hpp"

// Simple game entity for server
struct ServerEntity {
    uint32_t id;
    EntityType type;
    float x, y;
    float vx, vy;
    uint8_t hp;
    uint8_t playerId; // For player entities
};

class GameServer {
public:
    GameServer(short port) : server_(port), nextEntityId_(1000), gameRunning_(false) {
        std::random_device rd;
        rng_.seed(rd());
    }

    void start() {
        server_.start();
        gameRunning_ = true;
        std::cout << "[GameServer] Started on port 12345" << std::endl;
    }

    void run() {
        auto lastUpdate = std::chrono::steady_clock::now();
        const float updateRate = 1.0f / 60.0f; // 60 FPS
        float enemySpawnTimer = 0.0f;
        const float enemySpawnInterval = 2.0f;

        while (gameRunning_) {
            auto now = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
            
            if (deltaTime >= updateRate) {
                lastUpdate = now;
                
                // Process incoming packets
                server_.process();
                processPackets();
                
                // Update game simulation
                updateEntities(deltaTime);
                
                // Spawn enemies
                enemySpawnTimer += deltaTime;
                if (enemySpawnTimer >= enemySpawnInterval) {
                    enemySpawnTimer = 0.0f;
                    spawnEnemy();
                }
                
                // Send world snapshot to all clients
                sendWorldSnapshot();
                
                // Check for timeouts
                server_.checkTimeouts();
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    void processPackets() {
        while (server_.hasReceivedPackets()) {
            auto [packet, sender] = server_.getNextReceivedPacket();
            
            auto type = static_cast<GamePacketType>(packet.header.type);
            
            switch (type) {
                case GamePacketType::CLIENT_HELLO:
                    handleClientHello(packet, sender);
                    break;
                case GamePacketType::CLIENT_INPUT:
                    handleClientInput(packet, sender);
                    break;
                case GamePacketType::CLIENT_DISCONNECT:
                    handleClientDisconnect(sender);
                    break;
                default:
                    break;
            }
        }
    }

    void handleClientHello(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        // Create player entity
        uint8_t playerId = nextPlayerId_++;
        
        ServerEntity player;
        player.id = nextEntityId_++;
        player.type = EntityType::ENTITY_PLAYER;
        player.x = 100.0f;
        player.y = 400.0f + (playerId * 50.0f); // Offset players vertically
        player.vx = 0.0f;
        player.vy = 0.0f;
        player.hp = 100;
        player.playerId = playerId;
        
        entities_[player.id] = player;
        playerEntities_[playerId] = player.id;
        
        std::cout << "[GameServer] Client connected. Player ID: " << (int)playerId 
                  << " Entity ID: " << player.id << std::endl;
        
        // Send SERVER_WELCOME
        NetworkPacket welcome(static_cast<uint16_t>(GamePacketType::SERVER_WELCOME));
        welcome.header.timestamp = getCurrentTimestamp();
        welcome.payload.push_back(playerId);
        
        server_.sendTo(welcome, sender);
        
        // Send ENTITY_SPAWN for the new player to all clients
        broadcastEntitySpawn(player);
    }

    void handleClientInput(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        if (packet.payload.size() < sizeof(ClientInput)) {
            return;
        }
        
        ClientInput input = ClientInput::deserialize(packet.payload.data());
        
        // Find player entity
        auto it = playerEntities_.find(input.playerId);
        if (it == playerEntities_.end()) {
            return;
        }
        
        uint32_t entityId = it->second;
        auto entityIt = entities_.find(entityId);
        if (entityIt == entities_.end()) {
            return;
        }
        
        ServerEntity& player = entityIt->second;
        
        // Apply input
        const float speed = 500.0f;
        player.vx = 0.0f;
        player.vy = 0.0f;
        
        if (input.inputMask & (1 << 0)) player.vy = -speed; // Up
        if (input.inputMask & (1 << 1)) player.vy = speed;  // Down
        if (input.inputMask & (1 << 2)) player.vx = -speed; // Left
        if (input.inputMask & (1 << 3)) player.vx = speed;  // Right
        
        // Fire
        if (input.inputMask & (1 << 4)) {
            spawnPlayerMissile(player);
        }
    }

    void handleClientDisconnect(const asio::ip::udp::endpoint& sender) {
        std::cout << "[GameServer] Client disconnected: " << sender << std::endl;
    }

    void updateEntities(float deltaTime) {
        std::vector<uint32_t> toRemove;
        
        for (auto& [id, entity] : entities_) {
            // Update position
            entity.x += entity.vx * deltaTime;
            entity.y += entity.vy * deltaTime;
            
            // Boundary checking
            if (entity.x < -100.0f || entity.x > 2000.0f || 
                entity.y < -100.0f || entity.y > 1180.0f) {
                toRemove.push_back(id);
            }
            
            // Check collisions (simple)
            if (entity.type == EntityType::ENTITY_PLAYER_MISSILE) {
                for (auto& [enemyId, enemy] : entities_) {
                    if (enemy.type == EntityType::ENTITY_MONSTER) {
                        if (checkCollision(entity, enemy)) {
                            toRemove.push_back(id);
                            toRemove.push_back(enemyId);
                            break;
                        }
                    }
                }
            }
        }
        
        // Remove entities
        for (uint32_t id : toRemove) {
            entities_.erase(id);
            broadcastEntityDestroy(id);
        }
    }

    bool checkCollision(const ServerEntity& a, const ServerEntity& b) {
        const float size = 50.0f;
        return (a.x < b.x + size && a.x + size > b.x &&
                a.y < b.y + size && a.y + size > b.y);
    }

    void spawnEnemy() {
        ServerEntity enemy;
        enemy.id = nextEntityId_++;
        enemy.type = EntityType::ENTITY_MONSTER;
        enemy.x = 1920.0f;
        enemy.y = 100.0f + (dist_(rng_) % 880);
        enemy.vx = -200.0f;
        enemy.vy = 0.0f;
        enemy.hp = 10;
        enemy.playerId = 0;
        
        entities_[enemy.id] = enemy;
        broadcastEntitySpawn(enemy);
        
        std::cout << "[GameServer] Spawned enemy " << enemy.id << std::endl;
    }

    void spawnPlayerMissile(const ServerEntity& player) {
        ServerEntity missile;
        missile.id = nextEntityId_++;
        missile.type = EntityType::ENTITY_PLAYER_MISSILE;
        missile.x = player.x + 50.0f;
        missile.y = player.y + 10.0f;
        missile.vx = 800.0f;
        missile.vy = 0.0f;
        missile.hp = 1;
        missile.playerId = player.playerId;
        
        entities_[missile.id] = missile;
    }

    void sendWorldSnapshot() {
        // Build snapshot
        SnapshotHeader header;
        header.entityCount = entities_.size();
        
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT));
        packet.header.timestamp = getCurrentTimestamp();
        
        // Add header
        auto headerData = header.serialize();
        packet.payload.insert(packet.payload.end(), headerData.begin(), headerData.end());
        
        // Add entities
        for (const auto& [id, entity] : entities_) {
            EntityState state;
            state.id = entity.id;
            state.type = entity.type;
            state.x = entity.x;
            state.y = entity.y;
            state.vx = entity.vx;
            state.vy = entity.vy;
            state.hp = entity.hp;
            
            auto stateData = state.serialize();
            packet.payload.insert(packet.payload.end(), stateData.begin(), stateData.end());
        }
        
        server_.broadcast(packet);
    }

    void broadcastEntitySpawn(const ServerEntity& entity) {
        EntityState state;
        state.id = entity.id;
        state.type = entity.type;
        state.x = entity.x;
        state.y = entity.y;
        state.vx = entity.vx;
        state.vy = entity.vy;
        state.hp = entity.hp;
        
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ENTITY_SPAWN));
        packet.header.timestamp = getCurrentTimestamp();
        packet.setPayload(state.serialize());
        
        server_.broadcast(packet);
    }

    void broadcastEntityDestroy(uint32_t entityId) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ENTITY_DESTROY));
        packet.header.timestamp = getCurrentTimestamp();
        
        std::vector<char> payload(sizeof(uint32_t));
        std::memcpy(payload.data(), &entityId, sizeof(uint32_t));
        packet.setPayload(payload);
        
        server_.broadcast(packet);
    }

    uint32_t getCurrentTimestamp() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }

private:
    NetworkServer server_;
    std::unordered_map<uint32_t, ServerEntity> entities_;
    std::unordered_map<uint8_t, uint32_t> playerEntities_; // playerId -> entityId
    uint32_t nextEntityId_;
    uint8_t nextPlayerId_ = 1;
    bool gameRunning_;
    std::mt19937 rng_;
    std::uniform_int_distribution<> dist_;
};

int main() {
    std::cout << "R-Type Server Starting..." << std::endl;

    try {
        GameServer server(12345);
        server.start();
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
