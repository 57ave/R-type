#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <random>
#include "network/NetworkServer.hpp"
#include "network/RTypeProtocol.hpp"
#include "engine/Clock.hpp"

// Simple game entity for server
struct ServerEntity {
    uint32_t id;
    EntityType type;
    float x, y;
    float vx, vy;
    uint8_t hp;
    uint8_t playerId; // For player entities
    uint8_t playerLine; // For player ship color (spritesheet line)
    float fireTimer = 0.0f; // For rate limiting
    
    // Extended fields for variety
    uint8_t chargeLevel = 0;    // For missiles (0 = normal, 1-5 = charge levels)
    uint8_t enemyType = 0;      // For enemies (0 = basic, 1 = zigzag, 2 = sine, etc.)
    uint8_t projectileType = 0; // For projectiles (0 = normal, 1 = charged, etc.)
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
        rtype::engine::Clock updateClock;
        rtype::engine::Clock snapshotClock;
        
        const float fixedDeltaTime = 1.0f / 60.0f; // 60 FPS simulation
        float enemySpawnTimer = 0.0f;
        const float enemySpawnInterval = 2.0f;
        
        const float snapshotRate = 1.0f / 30.0f; // 30 snapshots per second
        float accumulatedTime = 0.0f;

        while (gameRunning_) {
            float elapsed = updateClock.restart();
            accumulatedTime += elapsed;
            
            // Fixed timestep update loop
            while (accumulatedTime >= fixedDeltaTime) {
                accumulatedTime -= fixedDeltaTime;
                
                // Process incoming packets
                server_.process();
                processPackets();
                
                // Update game simulation with FIXED deltaTime
                updateEntities(fixedDeltaTime);
                
                // Spawn enemies
                enemySpawnTimer += fixedDeltaTime;
                if (enemySpawnTimer >= enemySpawnInterval) {
                    enemySpawnTimer = 0.0f;
                    spawnEnemy();
                }
                
                // Send world snapshot at reduced rate (30Hz)
                if (snapshotClock.getElapsedTime() >= snapshotRate) {
                    snapshotClock.restart();
                    sendWorldSnapshot();
                }
                
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
        player.y = 200.0f + (playerId * 150.0f); // Offset players vertically with more space
        player.vx = 0.0f;
        player.vy = 0.0f;
        player.hp = 100;
        player.playerId = playerId;
        player.playerLine = (playerId - 1) % 5; // Cycle through 5 different ship colors (lines 0-4)
        
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
        
        // Fire (with rate limiting)
        if ((input.inputMask & (1 << 4)) && player.fireTimer <= 0.0f) {
            spawnPlayerMissile(player, input.chargeLevel);
            player.fireTimer = 0.2f; // 0.2 second cooldown
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
            
            // Update fire timer
            if (entity.fireTimer > 0.0f) {
                entity.fireTimer -= deltaTime;
            }
            
            // Enemy shooting logic
            if (entity.type == EntityType::ENTITY_MONSTER && entity.fireTimer <= 0.0f) {
                // Enemies shoot periodically
                if (entity.x < 1800.0f && entity.x > 100.0f) { // Only shoot when on screen
                    spawnEnemyMissile(entity);
                    entity.fireTimer = 2.0f + (dist_(rng_) % 200) / 100.0f; // Random fire rate 2-4 seconds
                }
            }
            
            // Boundary checking for players
            if (entity.type == EntityType::ENTITY_PLAYER) {
                if (entity.x < 0) entity.x = 0;
                if (entity.y < 0) entity.y = 0;
                if (entity.x > 1820) entity.x = 1820;
                if (entity.y > 1030) entity.y = 1030;
            }
            
            // Boundary checking for others (remove if out of bounds)
            if (entity.type != EntityType::ENTITY_PLAYER) {
                if (entity.x < -100.0f || entity.x > 2000.0f || 
                    entity.y < -100.0f || entity.y > 1180.0f) {
                    toRemove.push_back(id);
                }
            }
            
            // Check collisions (simple)
            if (entity.type == EntityType::ENTITY_PLAYER_MISSILE) {
                for (auto& [enemyId, enemy] : entities_) {
                    if (enemy.type == EntityType::ENTITY_MONSTER) {
                        if (checkCollision(entity, enemy)) {
                            std::cout << "[GameServer] Missile " << id << " hit enemy " << enemyId << "!" << std::endl;
                            // Create explosion at enemy position
                            spawnExplosion(enemy.x, enemy.y);
                            toRemove.push_back(id);
                            toRemove.push_back(enemyId);
                            break;
                        }
                    }
                }
            }
            
            // Check enemy missile vs players
            if (entity.type == EntityType::ENTITY_MONSTER_MISSILE) {
                for (auto& [playerId, player] : entities_) {
                    if (player.type == EntityType::ENTITY_PLAYER) {
                        if (checkCollision(entity, player)) {
                            // Create small explosion at missile hit
                            spawnExplosion(entity.x, entity.y);
                            toRemove.push_back(id);
                            player.hp -= 10; // Damage player
                            if (player.hp <= 0) {
                                toRemove.push_back(playerId);
                            }
                            break;
                        }
                    }
                }
            }
            
            // Check enemy collision with players (crash damage)
            if (entity.type == EntityType::ENTITY_MONSTER) {
                for (auto& [playerId, player] : entities_) {
                    if (player.type == EntityType::ENTITY_PLAYER) {
                        if (checkCollision(entity, player)) {
                            // Create explosion at collision point
                            spawnExplosion(entity.x, entity.y);
                            toRemove.push_back(id); // Destroy enemy
                            player.hp -= 20; // Heavy damage to player
                            if (player.hp <= 0) {
                                toRemove.push_back(playerId);
                            }
                            break;
                        }
                    }
                }
            }
        }
        
        // Remove entities
        for (uint32_t id : toRemove) {
            auto it = entities_.find(id);
            if (it != entities_.end()) {
                std::cout << "[GameServer] 🗑️  Destroying entity " << id << " (type: " << (int)it->second.type << ")" << std::endl;
                entities_.erase(it);
                broadcastEntityDestroy(id);
            }
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
        
        // Random enemy type (0-5 for different enemy types)
        enemy.enemyType = dist_(rng_) % 6;
        
        // Adjust speed and HP based on enemy type
        switch (enemy.enemyType) {
            case 0: // Basic
                enemy.vx = -200.0f;
                enemy.hp = 10;
                break;
            case 1: // ZigZag (faster)
                enemy.vx = -250.0f;
                enemy.hp = 8;
                break;
            case 2: // Sine Wave
                enemy.vx = -180.0f;
                enemy.hp = 12;
                break;
            case 3: // Kamikaze (very fast, low HP)
                enemy.vx = -400.0f;
                enemy.hp = 5;
                break;
            case 4: // Turret (slow, high HP)
                enemy.vx = -100.0f;
                enemy.hp = 20;
                break;
            case 5: // Boss (rare, slow, very high HP)
                enemy.vx = -150.0f;
                enemy.hp = 50;
                break;
        }
        
        enemy.vy = 0.0f;
        enemy.playerId = 0;
        enemy.playerLine = 0; // Enemies don't use playerLine
        enemy.fireTimer = 1.0f + (dist_(rng_) % 200) / 100.0f; // Initial fire delay 1-3s
        
        entities_[enemy.id] = enemy;
        broadcastEntitySpawn(enemy);
        
        std::cout << "[GameServer] 👾 Spawned enemy " << enemy.id << " (type " << (int)enemy.enemyType 
                  << ") at (" << enemy.x << ", " << enemy.y << ")" << std::endl;
    }

    void spawnPlayerMissile(const ServerEntity& player, uint8_t chargeLevel = 0) {
        ServerEntity missile;
        missile.id = nextEntityId_++;
        missile.type = EntityType::ENTITY_PLAYER_MISSILE;
        missile.x = player.x + 50.0f;
        missile.y = player.y + 10.0f;
        missile.vx = chargeLevel > 0 ? 1500.0f : 800.0f; // Charged missiles are faster
        missile.vy = 0.0f;
        missile.hp = chargeLevel > 0 ? chargeLevel : 1; // Charged missiles have more HP/damage
        missile.playerId = player.playerId;
        missile.playerLine = 0; // Missiles don't use playerLine
        missile.chargeLevel = chargeLevel;
        missile.projectileType = chargeLevel > 0 ? 1 : 0; // 0 = normal, 1 = charged
        
        entities_[missile.id] = missile;
        broadcastEntitySpawn(missile);  // Broadcast to all clients
        
        std::cout << "[GameServer] Player " << (int)player.playerId << " fired missile " << missile.id 
                  << (chargeLevel > 0 ? " (CHARGED level " + std::to_string(chargeLevel) + ")" : "") << std::endl;
    }
    
    void spawnEnemyMissile(const ServerEntity& enemy) {
        ServerEntity missile;
        missile.id = nextEntityId_++;
        missile.type = EntityType::ENTITY_MONSTER_MISSILE;
        missile.x = enemy.x - 20.0f;
        missile.y = enemy.y + 10.0f;
        missile.vx = -400.0f; // Shoot left towards players
        missile.vy = 0.0f;
        missile.hp = 1;
        missile.playerId = 0;
        missile.playerLine = 0;
        
        entities_[missile.id] = missile;
        broadcastEntitySpawn(missile);
        
        std::cout << "[GameServer] Enemy " << enemy.id << " fired missile " << missile.id << " at (" << missile.x << ", " << missile.y << ")" << std::endl;
    }

    void spawnExplosion(float x, float y) {
        ServerEntity explosion;
        explosion.id = nextEntityId_++;
        explosion.type = EntityType::ENTITY_EXPLOSION;
        explosion.x = x;
        explosion.y = y;
        explosion.vx = 0.0f;
        explosion.vy = 0.0f;
        explosion.hp = 1;
        explosion.playerId = 0;
        explosion.playerLine = 0;
        
        entities_[explosion.id] = explosion;
        broadcastEntitySpawn(explosion);
        
        std::cout << "[GameServer] Created explosion " << explosion.id << " at (" << x << ", " << y << ")" << std::endl;
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
            state.playerLine = entity.playerLine;
            
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
        state.playerLine = entity.playerLine;
        state.chargeLevel = entity.chargeLevel;
        state.enemyType = entity.enemyType;
        state.projectileType = entity.projectileType;
        
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
