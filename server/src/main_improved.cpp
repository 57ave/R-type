#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <random>
#include <cmath>
#include <algorithm>
#include "network/NetworkServer.hpp"
#include "network/RTypeProtocol.hpp"
#include "engine/Clock.hpp"

// Hash function for asio::ip::udp::endpoint to use in unordered_map
namespace std {
    template<>
    struct hash<asio::ip::udp::endpoint> {
        size_t operator()(const asio::ip::udp::endpoint& ep) const {
            std::hash<std::string> hasher;
            return hasher(ep.address().to_string() + ":" + std::to_string(ep.port()));
        }
    };
}

// Simple game entity for server
struct ServerEntity {
    uint32_t id;
    EntityType type;
    float x, y;
    float vx, vy;
    int32_t hp;           // Internal HP (can be > 255 for bosses)
    uint8_t playerId; // For player entities
    uint8_t playerLine; // For player ship color (spritesheet line)
    float fireTimer = 0.0f; // For rate limiting
    float lifetime = -1.0f; // For temporary entities like explosions (-1 = permanent)
    uint32_t score = 0; // Player score (0 for non-players)
    
    // Extended fields for variety
    uint8_t chargeLevel = 0;    // For missiles (0 = normal, 1-5 = charge levels)
    uint8_t enemyType = 0;      // 0=bug, 1=fighter(zigzag), 2=kamikaze, 3=drone
    uint8_t projectileType = 0; // For projectiles (0 = normal, 1 = charged, etc.)
    
    // Movement pattern state
    float zigzagTimer = 0.0f;   // Timer for zigzag direction changes
    float baseVy = 0.0f;        // Original vy for zigzag pattern
    
    // Fire pattern: 0=straight, 1=aimed, 2=circle, 3=spread
    uint8_t firePattern = 0;
    float fireRate = 2.0f;      // Seconds between shots
    
    // Shield
    float shieldTimer = 0.0f;   // Remaining shield time (0 = no shield)
    
    // Module equipped by player: 0=none, 1=laser, 2=homing, 3=spread, 4=wave
    uint8_t moduleType = 0;
    
    // Wave motion for wave projectiles
    float waveTime = 0.0f;
    float waveAmplitude = 0.0f;
    float waveFrequency = 0.0f;
    
    // Homing missile fields
    uint32_t homingTarget = 0;
    float homingSpeed = 0.0f;
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
        eng::engine::Clock updateClock;
        eng::engine::Clock snapshotClock;
        
        const float fixedDeltaTime = 1.0f / 60.0f; // 60 FPS simulation
        
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
                
                // Spawn enemies ONLY if there's an active game (room in PLAYING state)
                bool hasActiveGame = false;
                auto rooms = server_.getRoomManager().getRooms();
                for (const auto& room : rooms) {
                    if (room.state == RoomState::PLAYING) {
                        hasActiveGame = true;
                        break;
                    }
                }
                
                if (hasActiveGame) {
                    updateLevelSystem(fixedDeltaTime);
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
    // ==========================================
    // LEVEL SYSTEM
    // ==========================================
    
    // Level configuration (hardcoded from Lua design)
    struct WaveEnemy {
        uint8_t type;       // 0=bug, 1=fighter, 2=kamikaze
        int count;
        float interval;     // seconds between spawns in this wave
    };
    
    struct Wave {
        float time;         // seconds since level start
        std::vector<WaveEnemy> enemies;
    };
    
    struct BossConfig {
        uint8_t type;       // enemyType (3=FirstBoss, 4=SecondBoss, 5=LastBoss)
        uint16_t health;
        float speed;
        float fireRate;
        uint8_t firePattern;
        float spawnTime;    // seconds since level start
    };
    
    struct LevelConfig {
        int id;
        std::string name;
        std::vector<uint8_t> enemyTypes;    // Allowed enemy types
        std::vector<uint8_t> moduleTypes;   // Allowed module types
        float enemyInterval;
        float powerupInterval;
        float moduleInterval;
        int maxEnemies;
        std::vector<Wave> waves;
        BossConfig boss;
        bool stopSpawningAtBoss;
    };
    
    // Level state
    int currentLevel_ = 1;
    float levelTimer_ = 0.0f;
    float enemySpawnTimer_ = 0.0f;
    float powerupSpawnTimer_ = 0.0f;
    float moduleSpawnTimer_ = 0.0f;
    int currentWaveIndex_ = 0;
    bool bossSpawned_ = false;
    uint32_t bossEntityId_ = 0;
    bool bossAlive_ = false;
    bool levelActive_ = false;
    uint8_t moduleRotationIdx_ = 0;

    // Wave spawn state
    struct WaveSpawnState {
        int enemyIdx = 0;       // Current enemy group index
        int spawnedCount = 0;   // How many spawned in current group
        float spawnTimer = 0.0f;
        bool active = false;
    };
    WaveSpawnState waveSpawnState_;
    
    LevelConfig getLevelConfig(int level) {
        LevelConfig config;
        config.stopSpawningAtBoss = true;
        
        switch (level) {
            case 1:
                config.id = 1;
                config.name = "First Contact";
                config.enemyTypes = {0};           // Only bugs
                config.moduleTypes = {3, 4};       // spread, wave (NO homing)
                config.enemyInterval = 2.5f;
                config.powerupInterval = 15.0f;
                config.moduleInterval = 25.0f;
                config.maxEnemies = 8;
                config.waves = {
                    {3.0f,  {{0, 3, 1.5f}}},
                    {15.0f, {{0, 5, 1.0f}}},
                    {30.0f, {{0, 6, 0.8f}}},
                    {50.0f, {{0, 8, 0.6f}}},
                    {70.0f, {{0, 10, 0.5f}}},
                };
                config.boss = {3, 1000, 80.0f, 2.0f, 0, 90.0f};  // FirstBoss
                break;
            case 2:
                config.id = 2;
                config.name = "Rising Threat";
                config.enemyTypes = {0, 1};        // Bugs + Bats
                config.moduleTypes = {3, 4};       // spread, wave (NO homing)
                config.enemyInterval = 2.0f;
                config.powerupInterval = 12.0f;
                config.moduleInterval = 22.0f;
                config.maxEnemies = 12;
                config.waves = {
                    {3.0f,  {{0, 3, 1.2f}, {1, 2, 1.5f}}},
                    {18.0f, {{0, 4, 0.8f}, {1, 3, 1.0f}}},
                    {35.0f, {{1, 5, 0.7f}, {0, 3, 1.0f}}},
                    {55.0f, {{0, 6, 0.5f}, {1, 4, 0.6f}}},
                    {75.0f, {{0, 8, 0.4f}, {1, 5, 0.5f}}},
                };
                config.boss = {4, 2000, 60.0f, 1.5f, 2, 95.0f};  // SecondBoss
                break;
            case 3:
            default:
                config.id = 3;
                config.name = "Final Assault";
                config.enemyTypes = {0, 1, 2};    // ALL enemies
                config.moduleTypes = {1, 3, 4};   // ALL modules including homing!
                config.enemyInterval = 1.5f;
                config.powerupInterval = 10.0f;
                config.moduleInterval = 20.0f;
                config.maxEnemies = 15;
                config.waves = {
                    {3.0f,  {{0, 4, 0.8f}, {1, 3, 1.0f}, {2, 2, 1.2f}}},
                    {18.0f, {{2, 5, 0.6f}, {0, 3, 0.8f}}},
                    {35.0f, {{0, 5, 0.5f}, {1, 4, 0.6f}, {2, 3, 0.7f}}},
                    {55.0f, {{0, 8, 0.3f}, {1, 5, 0.4f}, {2, 4, 0.5f}}},
                    {75.0f, {{0, 10, 0.3f}, {1, 6, 0.4f}, {2, 5, 0.4f}}},
                };
                config.boss = {5, 3000, 100.0f, 1.0f, 3, 95.0f}; // LastBoss
                break;
        }
        return config;
    }
    
    void startLevel(int level) {
        currentLevel_ = level;
        levelTimer_ = 0.0f;
        enemySpawnTimer_ = 0.0f;
        powerupSpawnTimer_ = 0.0f;
        moduleSpawnTimer_ = 0.0f;
        currentWaveIndex_ = 0;
        bossSpawned_ = false;
        bossEntityId_ = 0;
        bossAlive_ = false;
        levelActive_ = true;
        moduleRotationIdx_ = 0;
        waveSpawnState_ = WaveSpawnState{};
        
        auto config = getLevelConfig(level);
        std::cout << "[GameServer] 🎮 === LEVEL " << level << ": " << config.name << " ===" << std::endl;
        
        // Broadcast level change to all clients
        broadcastLevelChange(level);
    }
    
    void updateLevelSystem(float dt) {
        if (!levelActive_) {
            // Start level 1 if no level is active
            startLevel(currentLevel_);
            return;
        }
        
        levelTimer_ += dt;
        auto config = getLevelConfig(currentLevel_);
        
        // Count current enemies
        int enemyCount = 0;
        for (const auto& [id, entity] : entities_) {
            if (entity.type == EntityType::ENTITY_MONSTER) {
                enemyCount++;
            }
        }
        
        // Check if boss was killed → advance to next level
        if (bossSpawned_ && bossAlive_) {
            if (entities_.find(bossEntityId_) == entities_.end()) {
                bossAlive_ = false;
                std::cout << "[GameServer] 🏆 Boss defeated! Level " << currentLevel_ << " complete!" << std::endl;
                
                // Clear remaining enemies
                std::vector<uint32_t> toRemove;
                for (const auto& [id, entity] : entities_) {
                    if (entity.type == EntityType::ENTITY_MONSTER || 
                        entity.type == EntityType::ENTITY_MONSTER_MISSILE) {
                        toRemove.push_back(id);
                    }
                }
                for (uint32_t id : toRemove) {
                    entities_.erase(id);
                    broadcastEntityDestroy(id);
                }
                
                if (currentLevel_ < 3) {
                    // Advance to next level after a short delay
                    currentLevel_++;
                    levelActive_ = false; // Will restart on next tick
                    std::cout << "[GameServer] ⏭️ Advancing to Level " << currentLevel_ << "..." << std::endl;
                } else {
                    std::cout << "[GameServer] 🎉 ALL LEVELS COMPLETE! Game Won!" << std::endl;
                    // Restart from level 1
                    currentLevel_ = 1;
                    levelActive_ = false;
                }
                return;
            }
        }
        
        // Process wave spawning (ongoing wave)
        if (waveSpawnState_.active) {
            processWaveSpawning(dt, config);
        }
        
        // Check for new waves to trigger
        if (currentWaveIndex_ < (int)config.waves.size() && !waveSpawnState_.active) {
            if (levelTimer_ >= config.waves[currentWaveIndex_].time) {
                // Start this wave
                waveSpawnState_.active = true;
                waveSpawnState_.enemyIdx = 0;
                waveSpawnState_.spawnedCount = 0;
                waveSpawnState_.spawnTimer = 0.0f;
                std::cout << "[GameServer] 🌊 Wave " << (currentWaveIndex_ + 1) 
                          << " triggered at " << levelTimer_ << "s" << std::endl;
            }
        }
        
        // Spawn boss when time comes
        if (!bossSpawned_ && levelTimer_ >= config.boss.spawnTime) {
            spawnBoss(config.boss);
            bossSpawned_ = true;
            bossAlive_ = true;
            std::cout << "[GameServer] 👹 BOSS SPAWNED! (Level " << currentLevel_ << ")" << std::endl;
        }
        
        // Regular spawning between waves (only if boss hasn't spawned or stopSpawningAtBoss is false)
        bool canSpawnRegular = !(bossSpawned_ && config.stopSpawningAtBoss);
        
        if (canSpawnRegular && enemyCount < config.maxEnemies) {
            enemySpawnTimer_ += dt;
            if (enemySpawnTimer_ >= config.enemyInterval) {
                enemySpawnTimer_ = 0.0f;
                spawnLevelEnemy(config);
            }
        }
        
        // Spawn powerups
        powerupSpawnTimer_ += dt;
        if (powerupSpawnTimer_ >= config.powerupInterval) {
            powerupSpawnTimer_ = 0.0f;
            spawnPowerup();
        }
        
        // Spawn modules (only allowed types for this level)
        moduleSpawnTimer_ += dt;
        if (moduleSpawnTimer_ >= config.moduleInterval) {
            moduleSpawnTimer_ = 0.0f;
            uint8_t modType = config.moduleTypes[moduleRotationIdx_ % config.moduleTypes.size()];
            spawnModule(modType);
            moduleRotationIdx_++;
        }
    }
    
    void processWaveSpawning(float dt, const LevelConfig& config) {
        if (currentWaveIndex_ >= (int)config.waves.size()) {
            waveSpawnState_.active = false;
            return;
        }
        
        const Wave& wave = config.waves[currentWaveIndex_];
        
        waveSpawnState_.spawnTimer += dt;
        
        // Find current enemy group
        if (waveSpawnState_.enemyIdx >= (int)wave.enemies.size()) {
            // Wave complete
            waveSpawnState_.active = false;
            currentWaveIndex_++;
            return;
        }
        
        const WaveEnemy& group = wave.enemies[waveSpawnState_.enemyIdx];
        
        if (waveSpawnState_.spawnTimer >= group.interval) {
            waveSpawnState_.spawnTimer = 0.0f;
            spawnEnemyOfType(group.type);
            waveSpawnState_.spawnedCount++;
            
            if (waveSpawnState_.spawnedCount >= group.count) {
                // Move to next enemy group
                waveSpawnState_.enemyIdx++;
                waveSpawnState_.spawnedCount = 0;
            }
        }
    }
    
    void spawnLevelEnemy(const LevelConfig& config) {
        // Pick random allowed enemy type for this level
        uint8_t enemyType = config.enemyTypes[dist_(rng_) % config.enemyTypes.size()];
        spawnEnemyOfType(enemyType);
    }
    
    void spawnEnemyOfType(uint8_t enemyType) {
        ServerEntity enemy;
        enemy.id = nextEntityId_++;
        enemy.type = EntityType::ENTITY_MONSTER;
        enemy.x = 1920.0f;
        enemy.y = 100.0f + (dist_(rng_) % 880);
        enemy.playerId = 0;
        enemy.playerLine = 0;
        
        switch (enemyType) {
            case 0: // Bug - straight, slow, straight fire
                enemy.enemyType = 0;
                enemy.vx = -400.0f;
                enemy.vy = 0.0f;
                enemy.hp = 10;
                enemy.firePattern = 0; // straight
                enemy.fireRate = 2.0f;
                break;
            case 1: // Fighter/Bat - zigzag, circle fire
                enemy.enemyType = 1;
                enemy.vx = -350.0f;
                enemy.vy = 80.0f;
                enemy.baseVy = 80.0f;
                enemy.hp = 30;
                enemy.firePattern = 2; // circle
                enemy.fireRate = 1.5f;
                break;
            case 2: // Kamikaze - rushes player, no fire
                enemy.enemyType = 2;
                enemy.vx = -500.0f;
                enemy.vy = 0.0f;
                enemy.hp = 20;
                enemy.firePattern = 255; // no fire
                enemy.fireRate = 999.0f;
                break;
            default:
                enemy.enemyType = 0;
                enemy.vx = -400.0f;
                enemy.vy = 0.0f;
                enemy.hp = 10;
                enemy.firePattern = 0;
                enemy.fireRate = 2.0f;
                break;
        }
        
        enemy.fireTimer = 1.0f + (dist_(rng_) % 200) / 100.0f;
        
        entities_[enemy.id] = enemy;
        broadcastEntitySpawn(enemy);
    }
    
    void spawnBoss(const BossConfig& bossConfig) {
        ServerEntity boss;
        boss.id = nextEntityId_++;
        boss.type = EntityType::ENTITY_MONSTER;
        boss.x = 1920.0f;
        boss.y = 400.0f; // Spawn at center-ish height
        boss.vx = -bossConfig.speed;
        boss.vy = 0.0f;
        boss.hp = bossConfig.health; // Full HP (internal int32_t)
        boss.playerId = 0;
        boss.playerLine = 0;
        boss.enemyType = bossConfig.type; // 3=FirstBoss, 4=SecondBoss, 5=LastBoss
        boss.firePattern = bossConfig.firePattern;
        boss.fireRate = bossConfig.fireRate;
        boss.fireTimer = 1.0f;
        
        bossEntityId_ = boss.id;
        
        entities_[boss.id] = boss;
        broadcastEntitySpawn(boss);
        
        std::cout << "[GameServer] 👹 Boss " << (int)bossConfig.type 
                  << " spawned (HP=" << (int)boss.hp << ")" << std::endl;
    }
    
    void broadcastLevelChange(int level) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::LEVEL_CHANGE));
        uint8_t levelId = static_cast<uint8_t>(level);
        packet.payload.push_back(static_cast<char>(levelId));
        
        // Broadcast to all rooms
        auto& roomManager = server_.getRoomManager();
        auto allRooms = roomManager.getAllRooms();
        for (auto& [roomId, room] : allRooms) {
            if (room->state == RoomState::PLAYING) {
                broadcastToRoom(roomId, packet);
            }
        }
        
        std::cout << "[GameServer] 📡 Broadcast LEVEL_CHANGE: Level " << level << std::endl;
    }

    void processPackets() {
        while (server_.hasReceivedPackets()) {
            auto [packet, sender] = server_.getNextReceivedPacket();
            
            auto type = static_cast<GamePacketType>(packet.header.type);
            
            switch (type) {
                case GamePacketType::CLIENT_HELLO:
                    handleClientHello(packet, sender);
                    break;
                case GamePacketType::CLIENT_TOGGLE_PAUSE:
                    handleClientTogglePause(packet, sender);
                    break;
                case GamePacketType::CLIENT_INPUT:
                    handleClientInput(packet, sender);
                    break;
                case GamePacketType::CLIENT_PING:
                    handleClientPing(packet, sender);
                    break;
                case GamePacketType::CLIENT_DISCONNECT:
                    handleClientDisconnect(sender);
                    break;
                    
                // Rooming system packets
                case GamePacketType::ROOM_LIST:
                    handleRoomListRequest(sender);
                    break;
                case GamePacketType::CREATE_ROOM:
                    handleCreateRoom(packet, sender);
                    break;
                case GamePacketType::JOIN_ROOM:
                    handleJoinRoom(packet, sender);
                    break;
                case GamePacketType::ROOM_LEAVE:
                    handleLeaveRoom(packet, sender);
                    break;
                case GamePacketType::PLAYER_READY:
                    handlePlayerReady(packet, sender);
                    break;
                case GamePacketType::GAME_START:
                    handleGameStart(packet, sender);
                    break;
                case GamePacketType::CHAT_MESSAGE:
                    handleChatMessage(packet, sender);
                    break;
                    
                default:
                    std::cout << "[GameServer] Unknown packet type: " << packet.header.type << std::endl;
                    break;
            }
        }
    }

    void handleClientHello(const NetworkPacket&, const asio::ip::udp::endpoint& sender) {
        // Assign player ID but don't create game entity yet
        // Entity will be created when the room starts (GAME_START)
        uint8_t playerId = nextPlayerId_++;
        
        endpointToPlayerId_[sender] = playerId; // Map endpoint to playerId
        
        std::cout << "[GameServer] Client connected. Assigned Player ID: " << (int)playerId 
                  << " (entity will be created when game starts)" << std::endl;
        
        // Send SERVER_WELCOME
        NetworkPacket welcome(static_cast<uint16_t>(GamePacketType::SERVER_WELCOME));
        welcome.header.timestamp = getCurrentTimestamp();
        welcome.payload.push_back(playerId);
        
        server_.sendTo(welcome, sender);
        std::cout << "[Network] Welcome sent to " << sender.address().to_string() 
                  << ":" << sender.port() << " (Player ID: " << (int)playerId << ")" << std::endl;
        
        // Don't create entity or broadcast yet - wait for game to start
    }

    void handleClientInput(const NetworkPacket& packet, const asio::ip::udp::endpoint&) {
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
        
        // Fire logic: shoot ONLY on release
        bool firePressed = (input.inputMask & (1 << 4)) != 0;
        bool prevFire = playerPrevFire_[input.playerId];
        
        if (firePressed) {
            // Track charge level while holding
            playerLastCharge_[input.playerId] = input.chargeLevel;
        } else if (prevFire && !firePressed) {
            // Released fire button
            uint8_t charge = playerLastCharge_[input.playerId];
            if (player.fireTimer <= 0.0f) {
                if (player.moduleType > 0) {
                    // Fire with module pattern
                    fireModuleMissile(player);
                    player.fireTimer = 0.2f;
                } else {
                    // Normal/charged shot
                    spawnPlayerMissile(player, charge);
                    player.fireTimer = charge > 0 ? 0.3f : 0.15f;
                }
            }
            playerLastCharge_[input.playerId] = 0;
        }
        playerPrevFire_[input.playerId] = firePressed;
    }

    // ✅ NOUVEAU: Handler pour CLIENT_PING
    void handleClientPing(const NetworkPacket&, const asio::ip::udp::endpoint& sender) {
        auto session = server_.getSession(sender);
        if (session) {
            // Update last packet time to prevent timeout
            session->updateLastPacketTime();
            
            // Send PING_REPLY
            NetworkPacket reply(static_cast<uint16_t>(GamePacketType::SERVER_PING_REPLY));
            reply.header.timestamp = getCurrentTimestamp();
            server_.sendTo(reply, sender);
        }
    }

    void handleClientDisconnect(const asio::ip::udp::endpoint& sender) {
        std::cout << "[GameServer] Client disconnected: " << sender << std::endl;
        
        // Try to get session first (from room-system-improvements)
        auto session = server_.getSession(sender);
        uint8_t playerId = 0;
        uint32_t roomId = 0;
        
        if (session) {
            // Use session-based approach (preferred method from room-system-improvements)
            playerId = session->playerId;
            roomId = session->roomId;
            std::cout << "[GameServer] Cleaning up player " << (int)playerId << " from session (room: " << roomId << ")" << std::endl;
        } else {
            // Fallback to endpoint mapping (from game_menu)
            auto epIt = endpointToPlayerId_.find(sender);
            if (epIt == endpointToPlayerId_.end()) {
                std::cout << "[GameServer] Unknown endpoint, cannot cleanup" << std::endl;
                return; // Unknown endpoint
            }
            playerId = epIt->second;
            std::cout << "[GameServer] Cleaning up player " << (int)playerId << " from endpoint mapping" << std::endl;
        }
        
        // ✅ NOUVEAU: Remove player entity with explosion
        auto playerIt = playerEntities_.find(playerId);
        if (playerIt != playerEntities_.end()) {
            uint32_t entityId = playerIt->second;
            
            // Create explosion at player position before destroying
            auto entityIt = entities_.find(entityId);
            if (entityIt != entities_.end()) {
                spawnExplosion(entityIt->second.x, entityIt->second.y);
                std::cout << "[GameServer] Created explosion at player " << (int)playerId << " position (" 
                          << entityIt->second.x << ", " << entityIt->second.y << ")" << std::endl;
            }
            
            // Remove from entities map
            if (entities_.erase(entityId)) {
                // Use the improved broadcastEntityDestroy method
                broadcastEntityDestroy(entityId);
                std::cout << "[GameServer] Removed player " << (int)playerId << " entity " << entityId << std::endl;
            }
            
            playerEntities_.erase(playerIt);
        }
        
        // ✅ NOUVEAU: Clean up room membership and transfer ownership if needed
        if (roomId != 0) {
            auto& roomManager = server_.getRoomManager();
            auto room = roomManager.getRoom(roomId);
            if (room) {
                room->removePlayer(playerId);
                std::cout << "[GameServer] Removed player " << (int)playerId << " from room " << roomId << std::endl;
                
                // ✅ If this was the host, transfer ownership to another player
                if (room->hostPlayerId == playerId && !room->playerIds.empty()) {
                    room->hostPlayerId = *room->playerIds.begin();
                    std::cout << "[GameServer] ⚡ Transferred host ownership of room " << roomId 
                              << " to player " << (int)room->hostPlayerId << std::endl;
                }
                
                // Broadcast updated player list
                broadcastRoomPlayers(roomId);
            }
        }
        
        // Clean up endpoint mapping (from game_menu)
        endpointToPlayerId_.erase(sender);
        
        // Remove the session from UDP server (from room-system-improvements)
        server_.removeClient(sender);
    }

    void updateEntities(float deltaTime) {
        std::vector<uint32_t> toRemove;
        
        for (auto& [id, entity] : entities_) {
            // ✅ Update lifetime for temporary entities (explosions, etc.)
            if (entity.lifetime > 0.0f) {
                entity.lifetime -= deltaTime;
                if (entity.lifetime <= 0.0f) {
                    toRemove.push_back(id);
                    std::cout << "[GameServer] Entity " << id << " (type: " << (int)entity.type << ") lifetime expired" << std::endl;
                    continue; // Skip rest of update for this entity
                }
            }
            
            // ✅ Explosions don't move, skip movement logic
            if (entity.type == EntityType::ENTITY_EXPLOSION) {
                continue;
            }
            
            // Update position
            entity.x += entity.vx * deltaTime;
            entity.y += entity.vy * deltaTime;
            
            // Wave projectile motion (sinusoidal Y)
            if (entity.type == EntityType::ENTITY_PLAYER_MISSILE && entity.projectileType == 5) {
                entity.waveTime += deltaTime;
                float angularFreq = entity.waveFrequency * 2.0f * 3.14159f;
                entity.vy = entity.waveAmplitude * angularFreq * std::cos(angularFreq * entity.waveTime);
            }
            
            // Homing projectile: track nearest enemy
            if (entity.type == EntityType::ENTITY_PLAYER_MISSILE && entity.projectileType == 3) {
                const ServerEntity* nearest = nullptr;
                float nearestDist = 600.0f; // Detection radius
                for (auto& [eid, e] : entities_) {
                    if (e.type != EntityType::ENTITY_MONSTER) continue;
                    float dx = e.x - entity.x;
                    float dy = e.y - entity.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist < nearestDist) {
                        nearestDist = dist;
                        nearest = &e;
                    }
                }
                if (nearest) {
                    float dx = nearest->x - entity.x;
                    float dy = nearest->y - entity.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist > 0.001f) {
                        float speed = entity.homingSpeed > 0.0f ? entity.homingSpeed : 500.0f;
                        // Smooth turn towards target
                        float targetVx = (dx / dist) * speed;
                        float targetVy = (dy / dist) * speed;
                        float turnRate = 5.0f * deltaTime; // Smooth turn
                        entity.vx += (targetVx - entity.vx) * turnRate;
                        entity.vy += (targetVy - entity.vy) * turnRate;
                        // Maintain speed
                        float currentSpeed = std::sqrt(entity.vx * entity.vx + entity.vy * entity.vy);
                        if (currentSpeed > 0.001f) {
                            entity.vx = (entity.vx / currentSpeed) * speed;
                            entity.vy = (entity.vy / currentSpeed) * speed;
                        }
                    }
                }
            }
            
            // Update fire timer
            if (entity.fireTimer > 0.0f) {
                entity.fireTimer -= deltaTime;
            }
            
            // Enemy shooting logic (with fire patterns)
            if (entity.type == EntityType::ENTITY_MONSTER && entity.fireTimer <= 0.0f) {
                // Only shoot when on screen and has a valid fire pattern
                if (entity.x < 1800.0f && entity.x > 100.0f && entity.firePattern != 255) {
                    spawnEnemyMissile(entity);
                    entity.fireTimer = entity.fireRate + (dist_(rng_) % 100) / 100.0f;
                }
            }
            
            // Zigzag pattern (fighter, enemyType=1): reverse vy periodically
            if (entity.type == EntityType::ENTITY_MONSTER && entity.enemyType == 1) {
                entity.zigzagTimer += deltaTime;
                if (entity.zigzagTimer >= 1.0f) {
                    entity.vy = -entity.vy;
                    entity.zigzagTimer = 0.0f;
                }
                // Bounce off top/bottom
                if (entity.y < 50.0f) entity.vy = std::abs(entity.baseVy);
                if (entity.y > 1000.0f) entity.vy = -std::abs(entity.baseVy);
            }
            
            // Kamikaze pattern (enemyType=2): rush towards nearest player
            if (entity.type == EntityType::ENTITY_MONSTER && entity.enemyType == 2) {
                const ServerEntity* nearestPlayer = findNearestPlayer(entity);
                if (nearestPlayer) {
                    float dx = nearestPlayer->x - entity.x;
                    float dy = nearestPlayer->y - entity.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist > 0.001f) {
                        float speed = 500.0f;
                        entity.vx = (dx / dist) * speed;
                        entity.vy = (dy / dist) * speed;
                    }
                }
            }
            
            // Boss movement pattern (enemyType >= 3): move to x=1500 then bob up/down
            if (entity.type == EntityType::ENTITY_MONSTER && entity.enemyType >= 3) {
                if (entity.x <= 1500.0f) {
                    entity.vx = 0.0f; // Stop horizontal movement
                    entity.x = 1500.0f;
                    // Bob up and down slowly
                    entity.zigzagTimer += deltaTime;
                    entity.vy = std::sin(entity.zigzagTimer * 1.5f) * 100.0f;
                }
                // Keep boss on screen
                if (entity.y < 50.0f) entity.y = 50.0f;
                if (entity.y > 900.0f) entity.y = 900.0f;
            }
            
            // Boundary checking for players
            if (entity.type == EntityType::ENTITY_PLAYER) {
                if (entity.x < 0) entity.x = 0;
                if (entity.y < 0) entity.y = 0;
                if (entity.x > 1820) entity.x = 1820;
                if (entity.y > 1030) entity.y = 1030;
                
                // Shield timer countdown
                if (entity.shieldTimer > 0.0f) {
                    entity.shieldTimer -= deltaTime;
                    entity.chargeLevel = 99; // Keep signaling shield to client
                    if (entity.shieldTimer <= 0.0f) {
                        entity.shieldTimer = 0.0f;
                        entity.chargeLevel = 0; // Shield expired
                        std::cout << "[GameServer] 🛡️ Shield expired for player " << (int)entity.playerId << std::endl;
                    }
                }
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
                            // Calculate damage based on missile charge level
                            int damage = 10; // base damage
                            if (entity.chargeLevel > 0) {
                                damage = entity.chargeLevel * 10; // charged shots do more
                            }
                            
                            enemy.hp -= damage;
                            toRemove.push_back(id); // Always destroy missile
                            
                            if (enemy.hp <= 0) {
                                // Enemy killed - award score
                                uint8_t shooterId = entity.playerId;
                                for (auto& [playerId, player] : entities_) {
                                    if (player.type == EntityType::ENTITY_PLAYER && player.playerId == shooterId) {
                                        uint32_t points = (enemy.enemyType >= 3) ? 500 : 100; // Boss = 500pts
                                        player.score += points;
                                        break;
                                    }
                                }
                                spawnExplosion(enemy.x, enemy.y);
                                toRemove.push_back(enemyId);
                            }
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
                            // No explosion when player is hit - just destroy missile
                            toRemove.push_back(id);
                            if (player.shieldTimer <= 0.0f) {
                                player.hp -= 10; // Damage player
                                if (player.hp <= 0) {
                                    toRemove.push_back(playerId);
                                }
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
                            // Create explosion at enemy position (enemy dies)
                            spawnExplosion(entity.x, entity.y);
                            toRemove.push_back(id); // Destroy enemy
                            if (player.shieldTimer <= 0.0f) {
                                player.hp -= 20; // Heavy damage to player
                                if (player.hp <= 0) {
                                    toRemove.push_back(playerId);
                                }
                            }
                            break;
                        }
                    }
                }
            }
            
            // Check powerup collision with players
            if (entity.type == EntityType::ENTITY_POWERUP) {
                for (auto& [playerId, player] : entities_) {
                    if (player.type == EntityType::ENTITY_PLAYER) {
                        if (checkCollision(entity, player)) {
                            toRemove.push_back(id); // Remove powerup
                            
                            if (entity.enemyType == 0) {
                                // ORANGE BOMB: destroy all visible enemies
                                std::cout << "[GameServer] 💥 Player " << (int)player.playerId << " picked up BOMB!" << std::endl;
                                for (auto& [eid, e] : entities_) {
                                    if (e.type == EntityType::ENTITY_MONSTER) {
                                        if (e.x >= -100.0f && e.x <= 2020.0f && e.y >= -100.0f && e.y <= 1180.0f) {
                                            spawnExplosion(e.x, e.y);
                                            toRemove.push_back(eid);
                                        }
                                    }
                                }
                            } else if (entity.enemyType == 1) {
                                // BLUE SHIELD: make player invulnerable for 10 seconds
                                std::cout << "[GameServer] 🛡️ Player " << (int)player.playerId << " picked up SHIELD!" << std::endl;
                                player.shieldTimer = 10.0f; // 10 seconds of invulnerability
                                player.chargeLevel = 99; // Signal to client that shield is active
                            }
                            break;
                        }
                    }
                }
            }
            
            // Check module collision with players (pickup)
            if (entity.type == EntityType::ENTITY_MODULE) {
                for (auto& [playerId, player] : entities_) {
                    if (player.type == EntityType::ENTITY_PLAYER) {
                        if (checkCollision(entity, player)) {
                            toRemove.push_back(id); // Remove module from world
                            player.moduleType = entity.enemyType; // 1=laser(homing), 3=spread, 4=wave
                            const char* names[] = {"", "laser(homing)", "", "spread", "wave"};
                            std::cout << "[GameServer] 🔧 Player " << (int)player.playerId 
                                      << " picked up module: " << names[entity.enemyType] << std::endl;
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

    // OLD spawnEnemy() replaced by level system's spawnEnemyOfType()

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
    
    void fireModuleMissile(const ServerEntity& player) {
        float baseSpeed = 800.0f;
        
        switch (player.moduleType) {
            case 1: { // LASER MODULE: fires homing missiles (tracks nearest enemy)
                ServerEntity missile;
                missile.id = nextEntityId_++;
                missile.type = EntityType::ENTITY_PLAYER_MISSILE;
                missile.x = player.x + 50.0f;
                missile.y = player.y + 10.0f;
                missile.vx = baseSpeed;
                missile.vy = 0.0f;
                missile.hp = 1;
                missile.playerId = player.playerId;
                missile.chargeLevel = 0;
                missile.projectileType = 3; // Homing projectile
                missile.homingSpeed = 500.0f;
                entities_[missile.id] = missile;
                broadcastEntitySpawn(missile);
                break;
            }
            case 3: { // SPREAD: fires 3 projectiles in a fan (-15°, 0°, +15°)
                const float angles[3] = {-0.2617f, 0.0f, 0.2617f};
                for (int i = 0; i < 3; ++i) {
                    ServerEntity missile;
                    missile.id = nextEntityId_++;
                    missile.type = EntityType::ENTITY_PLAYER_MISSILE;
                    missile.x = player.x + 50.0f;
                    missile.y = player.y + 10.0f;
                    missile.vx = baseSpeed * std::cos(angles[i]);
                    missile.vy = baseSpeed * std::sin(angles[i]);
                    missile.hp = 1;
                    missile.playerId = player.playerId;
                    missile.chargeLevel = 0;
                    missile.projectileType = 4; // Spread projectile
                    entities_[missile.id] = missile;
                    broadcastEntitySpawn(missile);
                }
                break;
            }
            case 4: { // WAVE: fires a projectile with sinusoidal motion
                ServerEntity missile;
                missile.id = nextEntityId_++;
                missile.type = EntityType::ENTITY_PLAYER_MISSILE;
                missile.x = player.x + 50.0f;
                missile.y = player.y + 10.0f;
                missile.vx = baseSpeed;
                missile.vy = 0.0f;
                missile.hp = 1;
                missile.playerId = player.playerId;
                missile.chargeLevel = 0;
                missile.projectileType = 5; // Wave projectile
                missile.waveTime = 0.0f;
                missile.waveAmplitude = 60.0f;
                missile.waveFrequency = 4.0f;
                entities_[missile.id] = missile;
                broadcastEntitySpawn(missile);
                break;
            }
            default:
                // Fallback: normal shot
                spawnPlayerMissile(player, 0);
                break;
        }
        
        const char* names[] = {"", "laser(homing)", "", "spread", "wave"};
        std::cout << "[GameServer] 🔧 Player " << (int)player.playerId 
                  << " fired with module: " << names[player.moduleType] << std::endl;
    }
    
    void spawnPowerup() {
        ServerEntity powerup;
        powerup.id = nextEntityId_++;
        powerup.type = EntityType::ENTITY_POWERUP;
        powerup.x = 1920.0f;
        powerup.y = 100.0f + (dist_(rng_) % 880);
        powerup.vx = -150.0f;
        powerup.vy = 0.0f;
        powerup.hp = 1;
        powerup.playerId = 0;
        powerup.playerLine = 0;
        
        // 50/50 orange or blue
        powerup.enemyType = (dist_(rng_) % 2 == 0) ? 0 : 1; // 0=orange, 1=blue
        
        entities_[powerup.id] = powerup;
        broadcastEntitySpawn(powerup);
        
        std::cout << "[GameServer] ⭐ Spawned powerup " << powerup.id 
                  << " (" << (powerup.enemyType == 0 ? "orange/bomb" : "blue/shield") 
                  << ") at (" << powerup.x << ", " << powerup.y << ")" << std::endl;
    }

    // moduleType: 1=laser(homing), 3=spread, 4=wave
    void spawnModule(uint8_t modType) {
        ServerEntity mod;
        mod.id = nextEntityId_++;
        mod.type = EntityType::ENTITY_MODULE;
        mod.x = 1920.0f;
        mod.y = 100.0f + (dist_(rng_) % 880);
        mod.vx = -100.0f;
        mod.vy = 0.0f;
        mod.hp = 1;
        mod.playerId = 0;
        mod.playerLine = 0;
        mod.enemyType = modType; // Reuse enemyType to identify module type for client
        
        entities_[mod.id] = mod;
        broadcastEntitySpawn(mod);
        
        const char* names[] = {"", "laser(homing)", "", "spread", "wave"};
        std::cout << "[GameServer] 🔧 Spawned module " << mod.id 
                  << " (" << names[modType] 
                  << ") at (" << mod.x << ", " << mod.y << ")" << std::endl;
    }

    void spawnEnemyMissile(const ServerEntity& enemy) {
        float projSpeed = std::abs(enemy.vx) * 1.5f; // Projectile toujours plus rapide que l'ennemi
        if (projSpeed < 400.0f) projSpeed = 400.0f;
        
        if (enemy.firePattern == 0) {
            // STRAIGHT: single shot to the left
            spawnSingleMissile(enemy, -projSpeed, 0.0f);
        } else if (enemy.firePattern == 1) {
            // AIMED: shoot towards nearest player
            const ServerEntity* target = findNearestPlayer(enemy);
            if (target) {
                float dx = target->x - enemy.x;
                float dy = target->y - enemy.y;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.001f) {
                    spawnSingleMissile(enemy, (dx / len) * projSpeed, (dy / len) * projSpeed);
                }
            } else {
                spawnSingleMissile(enemy, -projSpeed, 0.0f);
            }
        } else if (enemy.firePattern == 2) {
            // CIRCLE: 8 projectiles in all directions
            for (int i = 0; i < 8; ++i) {
                float angle = (2.0f * 3.14159f * i) / 8.0f;
                float circleSpeed = projSpeed * 0.8f;
                spawnSingleMissile(enemy, std::cos(angle) * circleSpeed, std::sin(angle) * circleSpeed);
            }
        } else if (enemy.firePattern == 3) {
            // SPREAD: 3 projectiles in a fan
            for (int i = -1; i <= 1; ++i) {
                float angle = i * 0.26f; // ~15 degrees
                float dx = -projSpeed * std::cos(angle);
                float dy = -projSpeed * std::sin(angle);
                spawnSingleMissile(enemy, dx, dy);
            }
        }
    }
    
    void spawnSingleMissile(const ServerEntity& enemy, float vx, float vy) {
        ServerEntity missile;
        missile.id = nextEntityId_++;
        missile.type = EntityType::ENTITY_MONSTER_MISSILE;
        missile.x = enemy.x - 40.0f;
        missile.y = enemy.y;
        missile.vx = vx;
        missile.vy = vy;
        missile.hp = 1;
        missile.playerId = 0;
        missile.playerLine = 0;
        
        entities_[missile.id] = missile;
        broadcastEntitySpawn(missile);
    }
    
    const ServerEntity* findNearestPlayer(const ServerEntity& from) {
        float nearestDist = 999999.0f;
        const ServerEntity* nearest = nullptr;
        for (const auto& [id, e] : entities_) {
            if (e.type == EntityType::ENTITY_PLAYER) {
                float dx = e.x - from.x;
                float dy = e.y - from.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < nearestDist) {
                    nearestDist = dist;
                    nearest = &e;
                }
            }
        }
        return nearest;
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
        explosion.lifetime = 0.5f; // Explosions disappear after 0.5 seconds
        
        entities_[explosion.id] = explosion;
        broadcastEntitySpawn(explosion);
        
        std::cout << "[GameServer] Created explosion " << explosion.id << " at (" << x << ", " << y << ") with lifetime " << explosion.lifetime << "s" << std::endl;
    }

    void sendWorldSnapshot() {
        auto& roomManager = server_.getRoomManager();
        auto allRooms = roomManager.getAllRooms();
        
        // Check if there are any rooms in PLAYING state
        bool hasPlayingRooms = false;
        for (auto& [roomId, room] : allRooms) {
            if (room->state == RoomState::PLAYING) {
                hasPlayingRooms = true;
                break;
            }
        }
        
        // MODE ROOMS: Envoyer un snapshot par room
        if (hasPlayingRooms) {
            for (auto& [roomId, room] : allRooms) {
                if (room->state != RoomState::PLAYING) continue;
                
                std::vector<const ServerEntity*> snapshotEntities;
                
                // Ajouter TOUS les joueurs de cette room
                for (uint8_t playerId : room->playerIds) {
                    auto it = playerEntities_.find(playerId);
                    if (it != playerEntities_.end()) {
                        uint32_t entityId = it->second;
                        auto entityIt = entities_.find(entityId);
                        if (entityIt != entities_.end()) {
                            snapshotEntities.push_back(&entityIt->second);
                        }
                    }
                }
                
                // Ajouter les autres entités (ennemis, projectiles, explosions, etc.)
                for (const auto& [id, entity] : entities_) {
                    if (entity.type == EntityType::ENTITY_PLAYER) continue;
                    snapshotEntities.push_back(&entity);
                }
                
                // Construire le packet
                SnapshotHeader header;
                header.entityCount = snapshotEntities.size();
                NetworkPacket packet(static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT));
                packet.header.timestamp = getCurrentTimestamp();
                
                auto headerData = header.serialize();
                packet.payload.insert(packet.payload.end(), headerData.begin(), headerData.end());
                
                for (const auto* entity : snapshotEntities) {
                    EntityState state;
                    state.id = entity->id;
                    state.type = entity->type;
                    state.x = entity->x;
                    state.y = entity->y;
                    state.vx = entity->vx;
                    state.vy = entity->vy;
                    state.hp = static_cast<uint8_t>(std::min(entity->hp, (int32_t)255));
                    state.playerLine = entity->playerLine;
                    state.playerId = entity->playerId; // include server-side playerId mapping
                    state.chargeLevel = entity->chargeLevel;
                    state.enemyType = entity->enemyType;
                    state.score = entity->score; // Send player score
                    // For players: send moduleType via projectileType field
                    if (entity->type == EntityType::ENTITY_PLAYER) {
                        state.projectileType = entity->moduleType;
                    } else {
                        state.projectileType = entity->projectileType;
                    }
                    
                    auto stateData = state.serialize();
                    packet.payload.insert(packet.payload.end(), stateData.begin(), stateData.end());
                }
                
                broadcastToRoom(roomId, packet);
            }
        } else {
            // MODE LOCAL/CLASSIQUE: Broadcast à tous (pas de rooms actives)
            std::vector<const ServerEntity*> snapshotEntities;
            for (const auto& [id, entity] : entities_) {
                if (entity.type == EntityType::ENTITY_EXPLOSION) continue;
                snapshotEntities.push_back(&entity);
            }
            
            SnapshotHeader header;
            header.entityCount = snapshotEntities.size();
            NetworkPacket packet(static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT));
            packet.header.timestamp = getCurrentTimestamp();
            
            auto headerData = header.serialize();
            packet.payload.insert(packet.payload.end(), headerData.begin(), headerData.end());
            
            for (const auto* entity : snapshotEntities) {
                EntityState state;
                state.id = entity->id;
                state.type = entity->type;
                state.x = entity->x;
                state.y = entity->y;
                state.vx = entity->vx;
                state.vy = entity->vy;
                state.hp = static_cast<uint8_t>(std::min(entity->hp, (int32_t)255));
                state.playerLine = entity->playerLine;
                state.playerId = entity->playerId;
                state.chargeLevel = entity->chargeLevel;
                state.enemyType = entity->enemyType;
                state.projectileType = entity->projectileType;
                state.score = entity->score; // Send player score
                
                auto stateData = state.serialize();
                packet.payload.insert(packet.payload.end(), stateData.begin(), stateData.end());
            }
            
            server_.broadcast(packet);
        }
    }

    void broadcastEntitySpawn(const ServerEntity& entity) {
        EntityState state;
        state.id = entity.id;
        state.type = entity.type;
        state.x = entity.x;
        state.y = entity.y;
        state.vx = entity.vx;
        state.vy = entity.vy;
        state.hp = static_cast<uint8_t>(std::min(entity.hp, (int32_t)255));
        state.playerLine = entity.playerLine;
        state.playerId = entity.playerId;
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

    // ========== ROOMING SYSTEM HANDLERS ==========
    
    void handleRoomListRequest(const asio::ip::udp::endpoint& sender) {
        auto rooms = server_.getRoomManager().getRooms();
        
        RoomListPayload payload;
        for (const auto& room : rooms) {
            RoomInfo info;
            info.id = room.id;
            info.name = room.name;
            info.currentPlayers = static_cast<uint8_t>(room.playerIds.size());
            info.maxPlayers = room.maxPlayers;
            payload.rooms.push_back(info);
        }
        
        NetworkPacket reply(static_cast<uint16_t>(GamePacketType::ROOM_LIST_REPLY));
        reply.setPayload(payload.serialize());
        reply.header.timestamp = getCurrentTimestamp();
        
        server_.sendTo(reply, sender);
        
        std::cout << "[GameServer] Sent room list (" << rooms.size() << " rooms) to " 
                  << sender << std::endl;
    }
    
    void handleCreateRoom(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        try {
            CreateRoomPayload payload = CreateRoomPayload::deserialize(packet.payload);
            
            // Find player ID from session
            auto session = server_.getSession(sender);
            if (!session) {
                std::cerr << "[GameServer] CREATE_ROOM from unknown client" << std::endl;
                return;
            }
            
            uint8_t playerId = session->playerId;
            uint32_t roomId = server_.getRoomManager().createRoom(
                payload.name, 
                payload.maxPlayers, 
                playerId  // host
            );
            
            // IMPORTANT: L'hôte doit rejoindre sa propre room !
            bool joined = server_.getRoomManager().joinRoom(roomId, playerId);
            if (joined) {
                session->roomId = roomId;
                playerToRoom_[playerId] = roomId;
            }
            
            std::cout << "[GameServer] Room '" << payload.name << "' created (ID: " 
                      << roomId << ") by player " << static_cast<int>(playerId) << std::endl;
            
            // Send ROOM_CREATED confirmation
            NetworkPacket createdReply(static_cast<uint16_t>(GamePacketType::ROOM_CREATED));
            Network::Serializer createdSerializer;
            createdSerializer.write(roomId);
            createdReply.setPayload(createdSerializer.getBuffer());
            createdReply.header.timestamp = getCurrentTimestamp();
            server_.sendTo(createdReply, sender);
            
            // Send ROOM_JOINED confirmation (pour que le client affiche le lobby)
            NetworkPacket joinedReply(static_cast<uint16_t>(GamePacketType::ROOM_JOINED));
            Network::Serializer joinedSerializer;
            joinedSerializer.write(roomId);
            joinedSerializer.writeString(payload.name);
            
            // Send maxPlayers and hostPlayerId so client knows full room info
            auto createdRoom = server_.getRoomManager().getRoom(roomId);
            if (createdRoom) {
                joinedSerializer.write(createdRoom->maxPlayers);
                joinedSerializer.write(static_cast<uint32_t>(createdRoom->hostPlayerId));
            } else {
                joinedSerializer.write(static_cast<uint8_t>(4));  // default
                joinedSerializer.write(static_cast<uint32_t>(playerId));  // fallback
            }
            
            joinedReply.setPayload(joinedSerializer.getBuffer());
            joinedReply.header.timestamp = getCurrentTimestamp();
            server_.sendTo(joinedReply, sender);
            
            // NOUVEAU: Broadcaster la liste des joueurs (l'hôte se verra maintenant)
            broadcastRoomPlayers(roomId);
            
            // Note: Pas de broadcast de room list ici - les clients font REFRESH manuellement
            
        } catch (const std::exception& e) {
            std::cerr << "[GameServer] Error creating room: " << e.what() << std::endl;
        }
    }
    
    void handleJoinRoom(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        try {
            JoinRoomPayload payload = JoinRoomPayload::deserialize(packet.payload);
            
            auto session = server_.getSession(sender);
            if (!session) {
                std::cerr << "[GameServer] JOIN_ROOM from unknown client" << std::endl;
                return;
            }
            
            uint8_t playerId = session->playerId;
            bool success = server_.getRoomManager().joinRoom(payload.roomId, playerId);
            
            if (success) {
                session->roomId = payload.roomId;
                playerToRoom_[playerId] = payload.roomId;
                
                std::cout << "[GameServer] Player " << static_cast<int>(playerId) 
                          << " joined room " << payload.roomId << std::endl;
                
                // Send confirmation to client
                NetworkPacket reply(static_cast<uint16_t>(GamePacketType::ROOM_JOINED));
                Network::Serializer serializer;
                serializer.write(payload.roomId);
                
                auto room = server_.getRoomManager().getRoom(payload.roomId);
                if (room) {
                    serializer.writeString(room->name);
                    serializer.write(room->maxPlayers);
                    serializer.write(static_cast<uint32_t>(room->hostPlayerId));
                } else {
                    serializer.writeString("Unknown Room");
                    serializer.write(static_cast<uint8_t>(4));
                    serializer.write(static_cast<uint32_t>(0));
                }
                
                reply.setPayload(serializer.getBuffer());
                reply.header.timestamp = getCurrentTimestamp();
                server_.sendTo(reply, sender);
                
                // NOUVEAU: Broadcast updated player list to all room members
                broadcastRoomPlayers(payload.roomId);
                
                // Note: Pas de broadcast de room list ici - les clients font REFRESH manuellement
                
            } else {
                std::cerr << "[GameServer] Failed to join room " << payload.roomId 
                          << " (room full or not found)" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "[GameServer] Error joining room: " << e.what() << std::endl;
        }
    }
    
    void handleLeaveRoom(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        auto session = server_.getSession(sender);
        if (!session) {
            std::cerr << "[GameServer] ROOM_LEAVE from unknown client" << std::endl;
            return;
        }
        
        uint32_t roomId = session->roomId;
        if (roomId == 0) {
            std::cout << "[GameServer] Player " << static_cast<int>(session->playerId) 
                      << " tried to leave but not in a room" << std::endl;
            return;
        }
        
        uint8_t playerId = session->playerId;
        
        std::cout << "[GameServer] Player " << static_cast<int>(playerId) 
                  << " leaving room " << roomId << std::endl;
        
        // Remove player from room (void return, always succeeds)
        server_.getRoomManager().leaveRoom(roomId, playerId);
        
        // Clear session room info
        session->roomId = 0;
        playerToRoom_.erase(playerId);
        
        // Broadcast updated player list to remaining room members
        broadcastRoomPlayers(roomId);
        
        // Note: Room cleanup (if empty) is handled by RoomManager
    }
    
    void handlePlayerReady(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        auto session = server_.getSession(sender);
        if (!session || session->roomId == 0) {
            std::cerr << "[GameServer] PLAYER_READY from player not in a room" << std::endl;
            return;
        }
        
        // Parse ready state from payload (1 byte: 0 or 1)
        bool ready = false;
        if (packet.payload.size() >= 1) {
            ready = (packet.payload[0] != 0);
        }
        
        uint8_t playerId = session->playerId;
        uint32_t roomId = session->roomId;
        
        // Update ready state in room manager
        bool success = server_.getRoomManager().setPlayerReady(roomId, playerId, ready);
        
        if (success) {
            std::cout << "[GameServer] Player " << static_cast<int>(playerId) 
                      << " in room " << roomId 
                      << " set ready: " << (ready ? "true" : "false") << std::endl;
            
            // Broadcast updated player list to all room members
            broadcastRoomPlayers(roomId);
        } else {
            std::cerr << "[GameServer] Failed to set ready state for player " 
                      << static_cast<int>(playerId) << " in room " << roomId << std::endl;
        }
    }
    
    void handleGameStart(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        auto session = server_.getSession(sender);
        if (!session || session->roomId == 0) {
            std::cerr << "[GameServer] GAME_START from player not in a room" << std::endl;
            return;
        }
        
        auto room = server_.getRoomManager().getRoom(session->roomId);
        if (!room) {
            std::cerr << "[GameServer] GAME_START: room not found" << std::endl;
            return;
        }
        
        // Verify that the sender is the host
        if (room->hostPlayerId != session->playerId) {
            std::cerr << "[GameServer] Non-host player " << static_cast<int>(session->playerId) 
                      << " tried to start game in room " << session->roomId << std::endl;
            return;
        }
        
        // Prevent double-start: check if game already started
        if (room->state == RoomState::PLAYING) {
            std::cout << "[GameServer] Game already started in room " << session->roomId 
                      << ", ignoring duplicate GAME_START" << std::endl;
            return;
        }
        
        // Check minimum player count (need at least 2 players in the room)
        if (room->playerIds.size() < 2) {
            std::cerr << "[GameServer] Cannot start game: only " << room->playerIds.size() 
                      << " player(s) in room (need at least 2)" << std::endl;
            return;
        }
        
        // Change room state to PLAYING
        room->state = RoomState::PLAYING;
        
        std::cout << "[GameServer] ========== GAME STARTING in room " << session->roomId 
                  << " ==========" << std::endl;
        std::cout << "[GameServer] Creating player entities for " << room->playerIds.size() 
                  << " players..." << std::endl;
        
        // Create player entities for all players in the room
        int playerIndex = 0;
        for (uint8_t playerId : room->playerIds) {
            // Create player entity
            ServerEntity player;
            player.id = nextEntityId_++;
            player.type = EntityType::ENTITY_PLAYER;
            player.x = 100.0f;
            player.y = 200.0f + (playerIndex * 200.0f); // Offset players vertically
            player.vx = 0.0f;
            player.vy = 0.0f;
            player.hp = 100;
            player.playerId = playerId;
            player.playerLine = playerIndex % 5; // Cycle through 5 different ship colors
            
            entities_[player.id] = player;
            playerEntities_[playerId] = player.id;
            
            std::cout << "[GameServer]   Created player entity " << player.id 
                      << " for player " << (int)playerId 
                      << " (line " << (int)player.playerLine << ") at (" 
                      << player.x << ", " << player.y << ")" << std::endl;
            
            playerIndex++;
        }
        
        // Broadcast GAME_START to all players in the room
        NetworkPacket gameStartPacket(static_cast<uint16_t>(GamePacketType::GAME_START));
        gameStartPacket.header.timestamp = getCurrentTimestamp();
        
        broadcastToRoom(session->roomId, gameStartPacket);
        
        // Send initial world snapshot to all players in the room
        // This ensures all players see each other from the start
        std::cout << "[GameServer] Sending initial world snapshot to all players..." << std::endl;
        sendWorldSnapshot();
        
        // Mark server as running game logic
        gameRunning_ = true;
    }

    void handleClientTogglePause(const NetworkPacket& /*packet*/, const asio::ip::udp::endpoint& sender) {
        auto session = server_.getSession(sender);
        if (!session || session->roomId == 0) {
            std::cerr << "[GameServer] CLIENT_TOGGLE_PAUSE from player not in a room" << std::endl;
            return;
        }

        auto room = server_.getRoomManager().getRoom(session->roomId);
        if (!room) return;

        // Only host can toggle pause
        if (room->hostPlayerId != session->playerId) {
            std::cerr << "[GameServer] Non-host player " << (int)session->playerId << " tried to toggle pause" << std::endl;
            return;
        }

        // Toggle between PLAYING and PAUSED
        if (room->state == RoomState::PLAYING) {
            room->state = RoomState::PAUSED;
            std::cout << "[GameServer] Room " << room->id << " paused by host " << (int)session->playerId << std::endl;
        } else if (room->state == RoomState::PAUSED) {
            room->state = RoomState::PLAYING;
            std::cout << "[GameServer] Room " << room->id << " resumed by host " << (int)session->playerId << std::endl;
        } else {
            // If not playing, ignore
            std::cout << "[GameServer] TogglePause ignored - room not playing" << std::endl;
            return;
        }

        // Broadcast SERVER_SET_PAUSE with payload (uint8_t paused)
        uint8_t pausedFlag = (room->state == RoomState::PAUSED) ? 1 : 0;
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::SERVER_SET_PAUSE));
        std::vector<char> payload(sizeof(uint8_t));
        std::memcpy(payload.data(), &pausedFlag, sizeof(uint8_t));
        packet.setPayload(payload);
        packet.header.timestamp = getCurrentTimestamp();
        broadcastToRoom(room->id, packet);
    }
    
    void broadcastToRoom(uint32_t roomId, const NetworkPacket& packet) {
        auto room = server_.getRoomManager().getRoom(roomId);
        if (!room) {
            std::cerr << "[GameServer] broadcastToRoom: room " << roomId << " not found" << std::endl;
            return;
        }
        
        int sentCount = 0;
        // Use server's session list to find active clients
        auto sessions = server_.getActiveSessions();
        for (const auto& session : sessions) {
            // Check if this session's player is in the room
            if (std::find(room->playerIds.begin(), room->playerIds.end(), session.playerId) != room->playerIds.end()) {
                server_.sendTo(packet, session.endpoint);
                sentCount++;
            }
        }
        
        std::cout << "[GameServer] Broadcast to room " << roomId << ": sent to " 
                  << sentCount << "/" << room->playerIds.size() << " players" << std::endl;
    }
    
    // NOUVEAU: Broadcast la liste des joueurs dans une room (Problème 2)
    void broadcastRoomPlayers(uint32_t roomId) {
        auto room = server_.getRoomManager().getRoom(roomId);
        if (!room) return;
        
        RoomPlayersPayload payload;
        payload.roomId = roomId;
        
        int playerIndex = 1;
        for (uint8_t playerId : room->playerIds) {
            PlayerInRoomInfo info;
            info.playerId = playerId;
            info.playerName = "Player " + std::to_string(playerIndex);
            info.isHost = (playerId == room->hostPlayerId);
            info.isReady = room->isPlayerReady(playerId);
            payload.players.push_back(info);
            playerIndex++;
        }
        
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ROOM_PLAYERS_UPDATE));
        packet.setPayload(payload.serialize());
        packet.header.timestamp = getCurrentTimestamp();
        
        broadcastToRoom(roomId, packet);
        
        std::cout << "[GameServer] Broadcasted player list to room " << roomId 
                  << " (" << payload.players.size() << " players)" << std::endl;
    }
    
    // NOUVEAU: Handler pour les messages de chat (Problème 4)
    void handleChatMessage(const NetworkPacket& packet, const asio::ip::udp::endpoint& sender) {
        try {
            auto session = server_.getSession(sender);
            if (!session || session->roomId == 0) {
                std::cerr << "[GameServer] CHAT_MESSAGE from player not in a room" << std::endl;
                return;
            }
            
            ChatMessagePayload payload = ChatMessagePayload::deserialize(packet.payload);
            payload.senderId = session->playerId;
            payload.senderName = "Player " + std::to_string(session->playerId);
            payload.roomId = session->roomId;
            
            std::cout << "[GameServer] Chat message from Player " << static_cast<int>(session->playerId) 
                      << " in room " << session->roomId << ": " << payload.message << std::endl;
            
            // Broadcast to all players in the room
            NetworkPacket broadcastPacket(static_cast<uint16_t>(GamePacketType::CHAT_MESSAGE));
            broadcastPacket.setPayload(payload.serialize());
            broadcastPacket.header.timestamp = getCurrentTimestamp();
            broadcastToRoom(session->roomId, broadcastPacket);
            
        } catch (const std::exception& e) {
            std::cerr << "[GameServer] Error handling chat message: " << e.what() << std::endl;
        }
    }

    uint32_t getCurrentTimestamp() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }

private:
    NetworkServer server_;
    std::unordered_map<uint32_t, ServerEntity> entities_;
    std::unordered_map<uint8_t, uint32_t> playerEntities_; // playerId -> entityId
    std::unordered_map<asio::ip::udp::endpoint, uint8_t> endpointToPlayerId_; // endpoint -> playerId
    std::unordered_map<uint8_t, uint32_t> playerToRoom_;  // playerId -> roomId (NEW for rooming)
    std::unordered_map<uint8_t, bool> playerPrevFire_;     // playerId -> was fire pressed last frame
    std::unordered_map<uint8_t, uint8_t> playerLastCharge_; // playerId -> last charge level while holding
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
