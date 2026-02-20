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
#include "ServerConfig.hpp"

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
    
    // Hitbox dimensions (pixel-accurate per entity type)
    float width = 50.0f;
    float height = 50.0f;
    
    // Collision cooldown (prevents taking damage every frame from boss overlap)
    float collisionCooldown = 0.0f;
};

// Per-room game state: each room has its own independent game simulation
struct RoomGameState {
    uint32_t roomId = 0;
    
    // Entities for this room only
    std::unordered_map<uint32_t, ServerEntity> entities;
    std::unordered_map<uint8_t, uint32_t> playerEntities; // playerId -> entityId
    std::unordered_map<uint8_t, bool> playerPrevFire;     // playerId -> was fire pressed last frame
    std::unordered_map<uint8_t, uint8_t> playerLastCharge; // playerId -> last charge level

    // Level system state
    int currentLevel = 1;
    float levelTimer = 0.0f;
    float enemySpawnTimer = 0.0f;
    float powerupSpawnTimer = 0.0f;
    float moduleSpawnTimer = 0.0f;
    int currentWaveIndex = 0;
    bool bossSpawned = false;
    uint32_t bossEntityId = 0;
    bool bossAlive = false;
    bool levelActive = false;
    uint8_t moduleRotationIdx = 0;
    
    // Wave spawn state
    struct WaveSpawnState {
        int enemyIdx = 0;
        int spawnedCount = 0;
        float spawnTimer = 0.0f;
        bool active = false;
    };
    WaveSpawnState waveSpawnState;
};

class GameServer {
public:
    GameServer(short port) : server_(port), nextEntityId_(1000), gameRunning_(false) {
        std::random_device rd;
        rng_.seed(rd());
        
        // Load configuration from Lua
        if (!ServerConfig::loadFromLua(cfg_, "assets/scripts/config/server_config.lua")) {
            std::cout << "[GameServer] ⚠️ Using default config values" << std::endl;
        }
    }

    void start() {
        server_.start();
        gameRunning_ = true;
        std::cout << "[GameServer] Started on port " << cfg_.server.port << std::endl;
    }

    void run() {
        eng::engine::Clock updateClock;
        eng::engine::Clock snapshotClock;
        
        const float fixedDeltaTime = 1.0f / static_cast<float>(cfg_.server.tickRate);
        
        const float snapshotRate = 1.0f / static_cast<float>(cfg_.server.snapshotRate);
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
                
                // Update each room's game state independently
                for (auto& [roomId, gs] : roomStates_) {
                    auto room = server_.getRoomManager().getRoom(roomId);
                    if (!room || room->state != RoomState::PLAYING) continue;
                    
                    updateEntities(fixedDeltaTime, gs);
                    updateLevelSystem(fixedDeltaTime, gs);
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
    
    // Level state is now per-room (in RoomGameState)
    
    LevelConfig getLevelConfig(int level) {
        LevelConfig config;
        config.stopSpawningAtBoss = true;
        
        // Try to use Lua-loaded config
        int idx = level - 1;
        if (idx >= 0 && idx < (int)cfg_.levels.size()) {
            const auto& ld = cfg_.levels[idx];
            config.id = ld.id;
            config.name = ld.name;
            config.enemyTypes = ld.enemyTypes;
            config.moduleTypes = ld.moduleTypes;
            config.enemyInterval = ld.enemyInterval;
            config.powerupInterval = ld.powerupInterval;
            config.moduleInterval = ld.moduleInterval;
            config.maxEnemies = ld.maxEnemies;
            config.stopSpawningAtBoss = ld.stopSpawningAtBoss;
            
            for (const auto& wd : ld.waves) {
                Wave w;
                w.time = wd.time;
                for (const auto& g : wd.groups) {
                    w.enemies.push_back({g.type, g.count, g.interval});
                }
                config.waves.push_back(w);
            }
            
            config.boss = {
                ld.boss.enemyType,
                ld.boss.health,
                ld.boss.speed,
                ld.boss.fireRate,
                ld.boss.firePattern,
                ld.boss.spawnTime
            };
            return config;
        }
        
        // Fallback defaults if Lua config not loaded for this level
        switch (level) {
            case 1:
                config.id = 1;
                config.name = "First Contact";
                config.enemyTypes = {0};
                config.moduleTypes = {3, 4};
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
                config.boss = {3, 1000, 80.0f, 2.0f, 0, 90.0f};
                break;
            case 2:
                config.id = 2;
                config.name = "Rising Threat";
                config.enemyTypes = {0, 1};
                config.moduleTypes = {3, 4};
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
                config.boss = {4, 2000, 60.0f, 1.5f, 2, 95.0f};
                break;
            case 3:
            default:
                config.id = 3;
                config.name = "Final Assault";
                config.enemyTypes = {0, 1, 2};
                config.moduleTypes = {1, 3, 4};
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
                config.boss = {5, 3000, 100.0f, 1.0f, 3, 95.0f};
                break;
        }
        return config;
    }
    
    void startLevel(int level, RoomGameState& gs) {
        gs.currentLevel = level;
        gs.levelTimer = 0.0f;
        gs.enemySpawnTimer = 0.0f;
        gs.powerupSpawnTimer = 0.0f;
        gs.moduleSpawnTimer = 0.0f;
        gs.currentWaveIndex = 0;
        gs.bossSpawned = false;
        gs.bossEntityId = 0;
        gs.bossAlive = false;
        gs.levelActive = true;
        gs.moduleRotationIdx = 0;
        gs.waveSpawnState = RoomGameState::WaveSpawnState{};
        
        auto config = getLevelConfig(level);
        std::cout << "[GameServer] 🎮 === LEVEL " << level << ": " << config.name << " === (room " << gs.roomId << ")" << std::endl;
        
        // Broadcast level change to this room only
        broadcastLevelChange(level, gs.roomId);
    }
    
    void updateLevelSystem(float dt, RoomGameState& gs) {
        if (!gs.levelActive) {
            // Start level 1 if no level is active
            startLevel(gs.currentLevel, gs);
            return;
        }
        
        gs.levelTimer += dt;
        auto config = getLevelConfig(gs.currentLevel);
        
        // Count current enemies
        int enemyCount = 0;
        for (const auto& [id, entity] : gs.entities) {
            if (entity.type == EntityType::ENTITY_MONSTER) {
                enemyCount++;
            }
        }
        
        // Check if boss was killed → advance to next level
        if (gs.bossSpawned && gs.bossAlive) {
            if (gs.entities.find(gs.bossEntityId) == gs.entities.end()) {
                gs.bossAlive = false;
                std::cout << "[GameServer] 🏆 Boss defeated! Level " << gs.currentLevel << " complete! (room " << gs.roomId << ")" << std::endl;
                
                // Clear remaining enemies
                std::vector<uint32_t> toRemove;
                for (const auto& [id, entity] : gs.entities) {
                    if (entity.type == EntityType::ENTITY_MONSTER || 
                        entity.type == EntityType::ENTITY_MONSTER_MISSILE) {
                        toRemove.push_back(id);
                    }
                }
                for (uint32_t id : toRemove) {
                    gs.entities.erase(id);
                    broadcastEntityDestroy(id, gs.roomId);
                }
                
                if (gs.currentLevel < cfg_.maxLevel) {
                    // Advance to next level after a short delay
                    gs.currentLevel++;
                    gs.levelActive = false; // Will restart on next tick
                    std::cout << "[GameServer] ⏭️ Advancing to Level " << gs.currentLevel << "... (room " << gs.roomId << ")" << std::endl;
                } else {
                    std::cout << "[GameServer] 🎉 ALL LEVELS COMPLETE! Game Won! (room " << gs.roomId << ")" << std::endl;
                    // Calculate total score from all players
                    uint32_t totalScore = 0;
                    for (const auto& [eid, e] : gs.entities) {
                        if (e.type == EntityType::ENTITY_PLAYER) {
                            totalScore += e.score;
                        }
                    }
                    broadcastGameVictory(totalScore, gs.roomId);
                    gs.levelActive = false;
                }
                return;
            }
        }
        
        // Process wave spawning (ongoing wave)
        if (gs.waveSpawnState.active) {
            processWaveSpawning(dt, config, gs);
        }
        
        // Check for new waves to trigger
        if (gs.currentWaveIndex < (int)config.waves.size() && !gs.waveSpawnState.active) {
            if (gs.levelTimer >= config.waves[gs.currentWaveIndex].time) {
                // Start this wave
                gs.waveSpawnState.active = true;
                gs.waveSpawnState.enemyIdx = 0;
                gs.waveSpawnState.spawnedCount = 0;
                gs.waveSpawnState.spawnTimer = 0.0f;
                std::cout << "[GameServer] 🌊 Wave " << (gs.currentWaveIndex + 1) 
                          << " triggered at " << gs.levelTimer << "s (room " << gs.roomId << ")" << std::endl;
            }
        }
        
        // Spawn boss when time comes
        if (!gs.bossSpawned && gs.levelTimer >= config.boss.spawnTime) {
            spawnBoss(config.boss, gs);
            gs.bossSpawned = true;
            gs.bossAlive = true;
            std::cout << "[GameServer] 👹 BOSS SPAWNED! (Level " << gs.currentLevel << ", room " << gs.roomId << ")" << std::endl;
        }
        
        // Regular spawning between waves (only if boss hasn't spawned or stopSpawningAtBoss is false)
        bool canSpawnRegular = !(gs.bossSpawned && config.stopSpawningAtBoss);
        
        if (canSpawnRegular && enemyCount < config.maxEnemies) {
            gs.enemySpawnTimer += dt;
            if (gs.enemySpawnTimer >= config.enemyInterval) {
                gs.enemySpawnTimer = 0.0f;
                spawnLevelEnemy(config, gs);
            }
        }
        
        // Spawn powerups
        gs.powerupSpawnTimer += dt;
        if (gs.powerupSpawnTimer >= config.powerupInterval) {
            gs.powerupSpawnTimer = 0.0f;
            spawnPowerup(gs);
        }
        
        // Spawn modules (only allowed types for this level)
        gs.moduleSpawnTimer += dt;
        if (gs.moduleSpawnTimer >= config.moduleInterval) {
            gs.moduleSpawnTimer = 0.0f;
            uint8_t modType = config.moduleTypes[gs.moduleRotationIdx % config.moduleTypes.size()];
            spawnModule(modType, gs);
            gs.moduleRotationIdx++;
        }
    }
    
    void processWaveSpawning(float dt, const LevelConfig& config, RoomGameState& gs) {
        if (gs.currentWaveIndex >= (int)config.waves.size()) {
            gs.waveSpawnState.active = false;
            return;
        }
        
        const Wave& wave = config.waves[gs.currentWaveIndex];
        
        gs.waveSpawnState.spawnTimer += dt;
        
        // Find current enemy group
        if (gs.waveSpawnState.enemyIdx >= (int)wave.enemies.size()) {
            // Wave complete
            gs.waveSpawnState.active = false;
            gs.currentWaveIndex++;
            return;
        }
        
        const WaveEnemy& group = wave.enemies[gs.waveSpawnState.enemyIdx];
        
        if (gs.waveSpawnState.spawnTimer >= group.interval) {
            gs.waveSpawnState.spawnTimer = 0.0f;
            spawnEnemyOfType(group.type, gs);
            gs.waveSpawnState.spawnedCount++;
            
            if (gs.waveSpawnState.spawnedCount >= group.count) {
                // Move to next enemy group
                gs.waveSpawnState.enemyIdx++;
                gs.waveSpawnState.spawnedCount = 0;
            }
        }
    }
    
    void spawnLevelEnemy(const LevelConfig& config, RoomGameState& gs) {
        // Pick random allowed enemy type for this level
        uint8_t enemyType = config.enemyTypes[dist_(rng_) % config.enemyTypes.size()];
        spawnEnemyOfType(enemyType, gs);
    }
    
    void spawnEnemyOfType(uint8_t enemyType, RoomGameState& gs) {
        ServerEntity enemy;
        enemy.id = nextEntityId_++;
        enemy.type = EntityType::ENTITY_MONSTER;
        enemy.x = cfg_.enemySpawn.spawnX;
        enemy.y = cfg_.enemySpawn.spawnYMin + (dist_(rng_) % cfg_.enemySpawn.spawnYRange);
        enemy.playerId = 0;
        enemy.playerLine = 0;
        
        switch (enemyType) {
            case 0: // Bug - straight, slow, straight fire
                enemy.enemyType = cfg_.bug.typeId;
                enemy.vx = cfg_.bug.vx;
                enemy.vy = cfg_.bug.vy;
                enemy.hp = cfg_.bug.health;
                enemy.firePattern = cfg_.bug.firePattern;
                enemy.fireRate = cfg_.bug.fireRate;
                enemy.width = 66.0f;   // 33*2.0 scale
                enemy.height = 58.0f;  // 29*2.0 scale
                break;
            case 1: // Fighter/Bat - zigzag, circle fire
                enemy.enemyType = cfg_.fighter.typeId;
                enemy.vx = cfg_.fighter.vx;
                enemy.vy = cfg_.fighter.vy;
                enemy.baseVy = cfg_.fighter.vy;
                enemy.hp = cfg_.fighter.health;
                enemy.firePattern = cfg_.fighter.firePattern;
                enemy.fireRate = cfg_.fighter.fireRate;
                enemy.width = 32.0f;   // 16*2.0 scale
                enemy.height = 26.0f;  // 13*2.0 scale
                break;
            case 2: // Kamikaze - rushes player, no fire
                enemy.enemyType = cfg_.kamikaze.typeId;
                enemy.vx = cfg_.kamikaze.vx;
                enemy.vy = cfg_.kamikaze.vy;
                enemy.hp = cfg_.kamikaze.health;
                enemy.firePattern = cfg_.kamikaze.firePattern;
                enemy.fireRate = cfg_.kamikaze.fireRate;
                enemy.width = 34.0f;   // 17*2.0 scale
                enemy.height = 36.0f;  // 18*2.0 scale
                break;
            default:
                enemy.enemyType = cfg_.bug.typeId;
                enemy.vx = cfg_.bug.vx;
                enemy.vy = cfg_.bug.vy;
                enemy.hp = cfg_.bug.health;
                enemy.firePattern = cfg_.bug.firePattern;
                enemy.fireRate = cfg_.bug.fireRate;
                enemy.width = 66.0f;
                enemy.height = 58.0f;
                break;
        }
        
        enemy.fireTimer = cfg_.enemySpawn.fireTimerBase + (dist_(rng_) % cfg_.enemySpawn.fireTimerRandomRange) / 100.0f;
        
        gs.entities[enemy.id] = enemy;
        broadcastEntitySpawn(enemy, gs.roomId);
    }
    
    void spawnBoss(const BossConfig& bossConfig, RoomGameState& gs) {
        ServerEntity boss;
        boss.id = nextEntityId_++;
        boss.type = EntityType::ENTITY_MONSTER;
        boss.x = cfg_.bossMovement.spawnX;
        boss.y = cfg_.bossMovement.spawnY;
        boss.vx = -bossConfig.speed;
        boss.vy = 0.0f;
        boss.hp = bossConfig.health;
        boss.playerId = 0;
        boss.playerLine = 0;
        boss.enemyType = bossConfig.type;
        boss.firePattern = bossConfig.firePattern;
        boss.fireRate = bossConfig.fireRate;
        boss.fireTimer = cfg_.enemySpawn.fireTimerBase;
        
        // Set boss hitbox based on type (sprite size * scale)
        switch (bossConfig.type) {
            case 3: // FirstBoss: 259x143 at 1.5x
                boss.width = 388.0f;
                boss.height = 214.0f;
                break;
            case 4: // SecondBoss: 161x211 at 1.5x
                boss.width = 241.0f;
                boss.height = 316.0f;
                break;
            case 5: // LastBoss: 81x71 at 2.5x
                boss.width = 202.0f;
                boss.height = 177.0f;
                break;
            default:
                boss.width = 200.0f;
                boss.height = 200.0f;
                break;
        }
        
        gs.bossEntityId = boss.id;
        
        gs.entities[boss.id] = boss;
        broadcastEntitySpawn(boss, gs.roomId);
        
        std::cout << "[GameServer] 👹 Boss " << (int)bossConfig.type 
                  << " spawned (HP=" << (int)boss.hp << ") in room " << gs.roomId << std::endl;
    }
    
    void broadcastLevelChange(int level, uint32_t roomId) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::LEVEL_CHANGE));
        uint8_t levelId = static_cast<uint8_t>(level);
        packet.payload.push_back(static_cast<char>(levelId));
        
        broadcastToRoom(roomId, packet);
        
        std::cout << "[GameServer] 📡 Broadcast LEVEL_CHANGE: Level " << level << " (room " << roomId << ")" << std::endl;
    }

    void broadcastGameOver(uint32_t totalScore, uint32_t roomId) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::GAME_OVER));
        packet.header.timestamp = getCurrentTimestamp();
        std::vector<char> payload(sizeof(uint32_t));
        std::memcpy(payload.data(), &totalScore, sizeof(uint32_t));
        packet.setPayload(payload);

        broadcastToRoom(roomId, packet);
        std::cout << "[GameServer] 💀 Broadcast GAME_OVER (score: " << totalScore << ") to room " << roomId << std::endl;
    }

    void broadcastGameVictory(uint32_t totalScore, uint32_t roomId) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::GAME_VICTORY));
        packet.header.timestamp = getCurrentTimestamp();
        std::vector<char> payload(sizeof(uint32_t));
        std::memcpy(payload.data(), &totalScore, sizeof(uint32_t));
        packet.setPayload(payload);

        broadcastToRoom(roomId, packet);
        std::cout << "[GameServer] 🏆 Broadcast GAME_VICTORY (score: " << totalScore << ") to room " << roomId << std::endl;
    }

    void processPackets() {
        while (server_.hasReceivedPackets()) {
            auto [packet, sender] = server_.getNextReceivedPacket();
            
            auto type = static_cast<GamePacketType>(packet.header.type);
            
            switch (type) {
                // Note: CLIENT_HELLO, CREATE_ROOM, JOIN_ROOM, ROOM_LIST are handled
                // by NetworkServer::process() at engine level and never reach here
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
                    // Silently ignore unknown types (many are handled at engine level)
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
            std::cerr << "[GameServer] INPUT: payload too small" << std::endl;
            return;
        }
        
        ClientInput input = ClientInput::deserialize(packet.payload.data());
        
        // Find which room this player belongs to
        auto roomIt = playerToRoom_.find(input.playerId);
        if (roomIt == playerToRoom_.end()) {
            std::cerr << "[GameServer] INPUT: player " << (int)input.playerId << " not in playerToRoom_ (map size: " << playerToRoom_.size() << ")" << std::endl;
            return;
        }
        
        auto gsIt = roomStates_.find(roomIt->second);
        if (gsIt == roomStates_.end()) {
            std::cerr << "[GameServer] INPUT: room " << roomIt->second << " not in roomStates_ (map size: " << roomStates_.size() << ")" << std::endl;
            return;
        }
        RoomGameState& gs = gsIt->second;
        
        // Find player entity in this room
        auto it = gs.playerEntities.find(input.playerId);
        if (it == gs.playerEntities.end()) {
            return;
        }
        
        uint32_t entityId = it->second;
        auto entityIt = gs.entities.find(entityId);
        if (entityIt == gs.entities.end()) {
            return;
        }
        
        ServerEntity& player = entityIt->second;
        
        // Apply input
        const float speed = cfg_.player.speed;
        player.vx = 0.0f;
        player.vy = 0.0f;
        
        if (input.inputMask & (1 << 0)) player.vy = -speed; // Up
        if (input.inputMask & (1 << 1)) player.vy = speed;  // Down
        if (input.inputMask & (1 << 2)) player.vx = -speed; // Left
        if (input.inputMask & (1 << 3)) player.vx = speed;  // Right
        
        // Fire logic: shoot ONLY on release
        bool firePressed = (input.inputMask & (1 << 4)) != 0;
        bool prevFire = gs.playerPrevFire[input.playerId];
        
        if (firePressed) {
            // Track charge level while holding
            gs.playerLastCharge[input.playerId] = input.chargeLevel;
        } else if (prevFire && !firePressed) {
            // Released fire button
            uint8_t charge = gs.playerLastCharge[input.playerId];
            if (player.fireTimer <= 0.0f) {
                if (player.moduleType > 0) {
                    // Fire with module pattern
                    fireModuleMissile(player, gs);
                    player.fireTimer = cfg_.modules.fireCooldown;
                } else {
                    // Normal/charged shot
                    spawnPlayerMissile(player, charge, gs);
                    player.fireTimer = charge > 0 ? cfg_.projectiles.player.fireCooldownCharged : cfg_.projectiles.player.fireCooldownNormal;
                }
            }
            gs.playerLastCharge[input.playerId] = 0;
        }
        gs.playerPrevFire[input.playerId] = firePressed;
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
        auto roomIt = playerToRoom_.find(playerId);
        if (roomIt != playerToRoom_.end()) {
            auto gsIt = roomStates_.find(roomIt->second);
            if (gsIt != roomStates_.end()) {
                RoomGameState& gs = gsIt->second;
                auto playerIt = gs.playerEntities.find(playerId);
                if (playerIt != gs.playerEntities.end()) {
                    uint32_t entityId = playerIt->second;
                    
                    // Create explosion at player position before destroying
                    auto entityIt = gs.entities.find(entityId);
                    if (entityIt != gs.entities.end()) {
                        spawnExplosion(entityIt->second.x, entityIt->second.y, gs);
                        std::cout << "[GameServer] Created explosion at player " << (int)playerId << " position (" 
                                  << entityIt->second.x << ", " << entityIt->second.y << ")" << std::endl;
                    }
                    
                    // Remove from entities map
                    if (gs.entities.erase(entityId)) {
                        broadcastEntityDestroy(entityId, gs.roomId);
                        std::cout << "[GameServer] Removed player " << (int)playerId << " entity " << entityId << std::endl;
                    }
                    
                    gs.playerEntities.erase(playerIt);
                    gs.playerPrevFire.erase(playerId);
                    gs.playerLastCharge.erase(playerId);
                }
            }
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
                
                // If room is now empty, clean up room game state
                if (room->playerIds.empty()) {
                    roomStates_.erase(roomId);
                    std::cout << "[GameServer] Cleaned up empty room state for room " << roomId << std::endl;
                }
            }
        }
        
        // Clean up endpoint mapping (from game_menu)
        endpointToPlayerId_.erase(sender);
        
        // Remove the session from UDP server (from room-system-improvements)
        server_.removeClient(sender);
    }

    void updateEntities(float deltaTime, RoomGameState& gs) {
        std::vector<uint32_t> toRemove;
        
        for (auto& [id, entity] : gs.entities) {
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
                float nearestDist = cfg_.modules.homing.detectionRadius;
                for (auto& [eid, e] : gs.entities) {
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
                        float speed = entity.homingSpeed > 0.0f ? entity.homingSpeed : cfg_.modules.homing.speed;
                        // Smooth turn towards target
                        float targetVx = (dx / dist) * speed;
                        float targetVy = (dy / dist) * speed;
                        float turnRate = cfg_.modules.homing.turnRate * deltaTime; // Smooth turn
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
                    spawnEnemyMissile(entity, gs);
                    entity.fireTimer = entity.fireRate + (dist_(rng_) % 100) / 100.0f;
                }
            }
            
            // Zigzag pattern (fighter, enemyType=1): reverse vy periodically
            if (entity.type == EntityType::ENTITY_MONSTER && entity.enemyType == 1) {
                entity.zigzagTimer += deltaTime;
                if (entity.zigzagTimer >= cfg_.fighter.zigzagInterval) {
                    entity.vy = -entity.vy;
                    entity.zigzagTimer = 0.0f;
                }
                // Bounce off top/bottom
                if (entity.y < cfg_.fighter.boundaryTop) entity.vy = std::abs(entity.baseVy);
                if (entity.y > cfg_.fighter.boundaryBottom) entity.vy = -std::abs(entity.baseVy);
            }
            
            // Kamikaze pattern (enemyType=2): rush towards nearest player
            if (entity.type == EntityType::ENTITY_MONSTER && entity.enemyType == 2) {
                const ServerEntity* nearestPlayer = findNearestPlayer(entity, gs);
                if (nearestPlayer) {
                    float dx = nearestPlayer->x - entity.x;
                    float dy = nearestPlayer->y - entity.y;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist > 0.001f) {
                        float speed = cfg_.kamikaze.trackingSpeed;
                        entity.vx = (dx / dist) * speed;
                        entity.vy = (dy / dist) * speed;
                    }
                }
            }
            
            // Boss movement pattern (enemyType >= 3): move to stop_x then bob up/down
            if (entity.type == EntityType::ENTITY_MONSTER && entity.enemyType >= 3) {
                if (entity.x <= cfg_.bossMovement.stopX) {
                    entity.vx = 0.0f; // Stop horizontal movement
                    entity.x = cfg_.bossMovement.stopX;
                    // Bob up and down slowly
                    entity.zigzagTimer += deltaTime;
                    entity.vy = std::sin(entity.zigzagTimer * cfg_.bossMovement.bobSpeed) * cfg_.bossMovement.bobAmplitude;
                }
                // Keep boss on screen
                if (entity.y < cfg_.bossMovement.boundaryTop) entity.y = cfg_.bossMovement.boundaryTop;
                if (entity.y > cfg_.bossMovement.boundaryBottom) entity.y = cfg_.bossMovement.boundaryBottom;
            }
            
            // Boundary checking for players
            if (entity.type == EntityType::ENTITY_PLAYER) {
                if (entity.x < cfg_.player.boundaryMinX) entity.x = cfg_.player.boundaryMinX;
                if (entity.y < cfg_.player.boundaryMinY) entity.y = cfg_.player.boundaryMinY;
                if (entity.x > cfg_.player.boundaryMaxX) entity.x = cfg_.player.boundaryMaxX;
                if (entity.y > cfg_.player.boundaryMaxY) entity.y = cfg_.player.boundaryMaxY;
                
                // Collision cooldown countdown
                if (entity.collisionCooldown > 0.0f) {
                    entity.collisionCooldown -= deltaTime;
                    if (entity.collisionCooldown < 0.0f) entity.collisionCooldown = 0.0f;
                }
                
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
                float margin = cfg_.collisions.oobMargin;
                if (entity.x < -margin || entity.x > cfg_.collisions.screenWidth + margin || 
                    entity.y < -margin || entity.y > cfg_.collisions.screenHeight + margin) {
                    toRemove.push_back(id);
                }
            }
            
            // Check collisions (simple)
            if (entity.type == EntityType::ENTITY_PLAYER_MISSILE) {
                for (auto& [enemyId, enemy] : gs.entities) {
                    if (enemy.type == EntityType::ENTITY_MONSTER) {
                        if (checkCollision(entity, enemy)) {
                            // Calculate damage based on missile charge level
                            int damage = cfg_.projectiles.player.baseDamage;
                            if (entity.chargeLevel > 0) {
                                damage = entity.chargeLevel * cfg_.projectiles.player.chargeDamageMultiplier;
                            }
                            
                            enemy.hp -= damage;
                            toRemove.push_back(id); // Always destroy missile
                            
                            if (enemy.hp <= 0) {
                                // Enemy killed - award score
                                uint8_t shooterId = entity.playerId;
                                for (auto& [playerId, player] : gs.entities) {
                                    if (player.type == EntityType::ENTITY_PLAYER && player.playerId == shooterId) {
                                        uint32_t points = (enemy.enemyType >= 3) ? cfg_.bossMovement.score : cfg_.bug.score; // Boss vs normal
                                        player.score += points;
                                        break;
                                    }
                                }
                                spawnExplosion(enemy.x, enemy.y, gs);
                                toRemove.push_back(enemyId);
                            }
                            break;
                        }
                    }
                }
            }
            
            // Check enemy missile vs players
            if (entity.type == EntityType::ENTITY_MONSTER_MISSILE) {
                for (auto& [playerId, player] : gs.entities) {
                    if (player.type == EntityType::ENTITY_PLAYER) {
                        if (checkCollision(entity, player)) {
                            // No explosion when player is hit - just destroy missile
                            toRemove.push_back(id);
                            if (player.shieldTimer <= 0.0f) {
                                player.hp -= cfg_.projectiles.missileDamage; // Damage player
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
                for (auto& [playerId, player] : gs.entities) {
                    if (player.type == EntityType::ENTITY_PLAYER) {
                        if (checkCollision(entity, player)) {
                            if (entity.enemyType >= 3) {
                                // Boss: mutual damage, don't destroy boss
                                // Use collision cooldown to prevent instant death from overlap
                                if (player.collisionCooldown <= 0.0f && player.shieldTimer <= 0.0f) {
                                    player.hp -= cfg_.bossMovement.collisionDamageToPlayer;
                                    player.collisionCooldown = 0.5f; // 500ms between collision damage ticks
                                    if (player.hp <= 0) {
                                        toRemove.push_back(playerId);
                                    }
                                }
                                entity.hp -= cfg_.bossMovement.collisionDamageFromPlayer;
                                if (entity.hp <= 0) {
                                    spawnExplosion(entity.x, entity.y, gs);
                                    toRemove.push_back(id);
                                }
                            } else {
                                // Normal enemy: destroy enemy, heavy damage to player
                                spawnExplosion(entity.x, entity.y, gs);
                                toRemove.push_back(id);
                                if (player.shieldTimer <= 0.0f) {
                                    player.hp -= cfg_.bug.collisionDamage;
                                    if (player.hp <= 0) {
                                        toRemove.push_back(playerId);
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
            
            // Check powerup collision with players
            if (entity.type == EntityType::ENTITY_POWERUP) {
                for (auto& [playerId, player] : gs.entities) {
                    if (player.type == EntityType::ENTITY_PLAYER) {
                        if (checkCollision(entity, player)) {
                            toRemove.push_back(id); // Remove powerup
                            
                            if (entity.enemyType == 0) {
                                // ORANGE BOMB: destroy all visible enemies, but only deal fraction HP to boss
                                std::cout << "[GameServer] 💥 Player " << (int)player.playerId << " picked up BOMB!" << std::endl;
                                float margin = cfg_.collisions.oobMargin;
                                for (auto& [eid, e] : gs.entities) {
                                    if (e.type == EntityType::ENTITY_MONSTER) {
                                        if (e.x >= -margin && e.x <= cfg_.collisions.screenWidth + margin && 
                                            e.y >= -margin && e.y <= cfg_.collisions.screenHeight + margin) {
                                            if (e.enemyType >= 3) {
                                                // Boss: deal fraction of boss config max HP
                                                auto bossConfig = getLevelConfig(gs.currentLevel).boss;
                                                int bossDamage = static_cast<int>(bossConfig.health * cfg_.powerups.orange.bossDamageFraction);
                                                e.hp -= bossDamage;
                                                std::cout << "[GameServer] 💥 Bomb dealt " << bossDamage << " to boss (HP: " << e.hp << ")" << std::endl;
                                                if (e.hp <= 0) {
                                                    spawnExplosion(e.x, e.y, gs);
                                                    toRemove.push_back(eid);
                                                }
                                            } else {
                                                spawnExplosion(e.x, e.y, gs);
                                                toRemove.push_back(eid);
                                            }
                                        }
                                    }
                                }
                            } else if (entity.enemyType == 1) {
                                // BLUE SHIELD: make player invulnerable
                                std::cout << "[GameServer] 🛡️ Player " << (int)player.playerId << " picked up SHIELD!" << std::endl;
                                player.shieldTimer = cfg_.powerups.blue.duration;
                                player.chargeLevel = 99; // Signal to client that shield is active
                            }
                            break;
                        }
                    }
                }
            }
            
            // Check module collision with players (pickup)
            if (entity.type == EntityType::ENTITY_MODULE) {
                for (auto& [playerId, player] : gs.entities) {
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
        
        // Compute total score BEFORE removing entities (dead players still have their score)
        uint32_t preRemoveTotalScore = 0;
        for (const auto& [eid, e] : gs.entities) {
            if (e.type == EntityType::ENTITY_PLAYER) {
                preRemoveTotalScore += e.score;
            }
        }

        // Remove entities
        for (uint32_t id : toRemove) {
            auto it = gs.entities.find(id);
            if (it != gs.entities.end()) {
                std::cout << "[GameServer] 🗑️  Destroying entity " << id << " (type: " << (int)it->second.type << ") in room " << gs.roomId << std::endl;
                gs.entities.erase(it);
                broadcastEntityDestroy(id, gs.roomId);
            }
        }

        // Check if all players are dead → GAME_OVER
        if (gs.levelActive) {
            bool anyPlayerAlive = false;
            for (const auto& [eid, e] : gs.entities) {
                if (e.type == EntityType::ENTITY_PLAYER) {
                    anyPlayerAlive = true;
                    break;
                }
            }
            // Also check playerEntities to see if any mapped player still exists
            if (!anyPlayerAlive && !gs.playerEntities.empty()) {
                bool foundAny = false;
                for (const auto& [pid, entityId] : gs.playerEntities) {
                    if (gs.entities.find(entityId) != gs.entities.end()) {
                        foundAny = true;
                        break;
                    }
                }
                if (!foundAny) {
                    std::cout << "[GameServer] 💀 All players dead! Game Over! Score: " << preRemoveTotalScore << " (room " << gs.roomId << ")" << std::endl;
                    broadcastGameOver(preRemoveTotalScore, gs.roomId);
                    gs.levelActive = false;
                }
            }
        }
    }

    bool checkCollision(const ServerEntity& a, const ServerEntity& b) {
        return (a.x < b.x + b.width && a.x + a.width > b.x &&
                a.y < b.y + b.height && a.y + a.height > b.y);
    }

    // OLD spawnEnemy() replaced by level system's spawnEnemyOfType()

    void spawnPlayerMissile(const ServerEntity& player, uint8_t chargeLevel, RoomGameState& gs) {
        ServerEntity missile;
        missile.id = nextEntityId_++;
        missile.type = EntityType::ENTITY_PLAYER_MISSILE;
        missile.x = player.x + cfg_.projectiles.player.spawnOffsetX;
        missile.y = player.y + cfg_.projectiles.player.spawnOffsetY;
        missile.vx = chargeLevel > 0 ? cfg_.projectiles.player.chargedSpeed : cfg_.projectiles.player.normalSpeed;
        missile.vy = 0.0f;
        missile.hp = chargeLevel > 0 ? chargeLevel : 1;
        missile.playerId = player.playerId;
        missile.playerLine = 0;
        missile.chargeLevel = chargeLevel;
        missile.projectileType = chargeLevel > 0 ? 1 : 0;
        missile.width = 60.0f;   // 20*3.0 scale
        missile.height = 60.0f;  // 20*3.0 scale
        
        gs.entities[missile.id] = missile;
        broadcastEntitySpawn(missile, gs.roomId);
        
        std::cout << "[GameServer] Player " << (int)player.playerId << " fired missile " << missile.id 
                  << (chargeLevel > 0 ? " (CHARGED level " + std::to_string(chargeLevel) + ")" : "") << std::endl;
    }
    
    void fireModuleMissile(const ServerEntity& player, RoomGameState& gs) {
        float baseSpeed = cfg_.modules.baseSpeed;
        
        switch (player.moduleType) {
            case 1: { // LASER MODULE: fires homing missiles (tracks nearest enemy)
                ServerEntity missile;
                missile.id = nextEntityId_++;
                missile.type = EntityType::ENTITY_PLAYER_MISSILE;
                missile.x = player.x + cfg_.projectiles.player.spawnOffsetX;
                missile.y = player.y + cfg_.projectiles.player.spawnOffsetY;
                missile.vx = baseSpeed;
                missile.vy = 0.0f;
                missile.hp = 1;
                missile.playerId = player.playerId;
                missile.chargeLevel = 0;
                missile.projectileType = cfg_.modules.homing.projectileType;
                missile.homingSpeed = cfg_.modules.homing.speed;
                missile.width = 60.0f;
                missile.height = 60.0f;
                gs.entities[missile.id] = missile;
                broadcastEntitySpawn(missile, gs.roomId);
                break;
            }
            case 3: { // SPREAD: fires projectiles in a fan
                for (size_t i = 0; i < cfg_.modules.spread.angles.size(); ++i) {
                    float angle = cfg_.modules.spread.angles[i];
                    ServerEntity missile;
                    missile.id = nextEntityId_++;
                    missile.type = EntityType::ENTITY_PLAYER_MISSILE;
                    missile.x = player.x + cfg_.projectiles.player.spawnOffsetX;
                    missile.y = player.y + cfg_.projectiles.player.spawnOffsetY;
                    missile.vx = baseSpeed * std::cos(angle);
                    missile.vy = baseSpeed * std::sin(angle);
                    missile.hp = 1;
                    missile.playerId = player.playerId;
                    missile.chargeLevel = 0;
                    missile.projectileType = cfg_.modules.spread.projectileType;
                    missile.width = 60.0f;
                    missile.height = 60.0f;
                    gs.entities[missile.id] = missile;
                    broadcastEntitySpawn(missile, gs.roomId);
                }
                break;
            }
            case 4: { // WAVE: fires a projectile with sinusoidal motion
                ServerEntity missile;
                missile.id = nextEntityId_++;
                missile.type = EntityType::ENTITY_PLAYER_MISSILE;
                missile.x = player.x + cfg_.projectiles.player.spawnOffsetX;
                missile.y = player.y + cfg_.projectiles.player.spawnOffsetY;
                missile.vx = baseSpeed;
                missile.vy = 0.0f;
                missile.hp = 1;
                missile.playerId = player.playerId;
                missile.chargeLevel = 0;
                missile.projectileType = cfg_.modules.wave.projectileType;
                missile.waveTime = 0.0f;
                missile.waveAmplitude = cfg_.modules.wave.amplitude;
                missile.waveFrequency = cfg_.modules.wave.frequency;
                missile.width = 60.0f;
                missile.height = 60.0f;
                gs.entities[missile.id] = missile;
                broadcastEntitySpawn(missile, gs.roomId);
                break;
            }
            default:
                // Fallback: normal shot
                spawnPlayerMissile(player, 0, gs);
                break;
        }
        
        const char* names[] = {"", "laser(homing)", "", "spread", "wave"};
        std::cout << "[GameServer] 🔧 Player " << (int)player.playerId 
                  << " fired with module: " << names[player.moduleType] << std::endl;
    }
    
    void spawnPowerup(RoomGameState& gs) {
        ServerEntity powerup;
        powerup.id = nextEntityId_++;
        powerup.type = EntityType::ENTITY_POWERUP;
        powerup.x = cfg_.powerups.spawnX;
        powerup.y = cfg_.powerups.spawnYMin + (dist_(rng_) % cfg_.powerups.spawnYRange);
        powerup.vx = cfg_.powerups.spawnVx;
        powerup.vy = 0.0f;
        powerup.hp = 1;
        powerup.playerId = 0;
        powerup.playerLine = 0;
        
        // 50/50 orange or blue
        powerup.enemyType = (dist_(rng_) % 2 == 0) ? 0 : 1; // 0=orange, 1=blue
        powerup.width = 122.0f;   // 612*0.2 scale
        powerup.height = 81.0f;   // 408*0.2 scale
        
        gs.entities[powerup.id] = powerup;
        broadcastEntitySpawn(powerup, gs.roomId);
        
        std::cout << "[GameServer] ⭐ Spawned powerup " << powerup.id 
                  << " (" << (powerup.enemyType == 0 ? "orange/bomb" : "blue/shield") 
                  << ") at (" << powerup.x << ", " << powerup.y << ") in room " << gs.roomId << std::endl;
    }

    // moduleType: 1=laser(homing), 3=spread, 4=wave
    void spawnModule(uint8_t modType, RoomGameState& gs) {
        ServerEntity mod;
        mod.id = nextEntityId_++;
        mod.type = EntityType::ENTITY_MODULE;
        mod.x = cfg_.enemySpawn.spawnX;
        mod.y = cfg_.enemySpawn.spawnYMin + (dist_(rng_) % cfg_.enemySpawn.spawnYRange);
        mod.vx = cfg_.modules.spawnVx;
        mod.vy = 0.0f;
        mod.hp = 1;
        mod.playerId = 0;
        mod.playerLine = 0;
        mod.enemyType = modType; // Reuse enemyType to identify module type for client
        mod.width = 68.0f;   // ~34*2.0 scale
        mod.height = 58.0f;  // ~29*2.0 scale
        
        gs.entities[mod.id] = mod;
        broadcastEntitySpawn(mod, gs.roomId);
        
        const char* names[] = {"", "laser(homing)", "", "spread", "wave"};
        std::cout << "[GameServer] 🔧 Spawned module " << mod.id 
                  << " (" << names[modType] 
                  << ") at (" << mod.x << ", " << mod.y << ") in room " << gs.roomId << std::endl;
    }

    void spawnEnemyMissile(const ServerEntity& enemy, RoomGameState& gs) {
        float projSpeed = std::abs(enemy.vx) * cfg_.projectiles.enemy.speedMultiplier;
        if (projSpeed < cfg_.projectiles.enemy.minSpeed) projSpeed = cfg_.projectiles.enemy.minSpeed;
        
        if (enemy.firePattern == 0) {
            // STRAIGHT: single shot to the left
            spawnSingleMissile(enemy, -projSpeed, 0.0f, gs);
        } else if (enemy.firePattern == 1) {
            // AIMED: shoot towards nearest player
            const ServerEntity* target = findNearestPlayer(enemy, gs);
            if (target) {
                float dx = target->x - enemy.x;
                float dy = target->y - enemy.y;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.001f) {
                    spawnSingleMissile(enemy, (dx / len) * projSpeed, (dy / len) * projSpeed, gs);
                }
            } else {
                spawnSingleMissile(enemy, -projSpeed, 0.0f, gs);
            }
        } else if (enemy.firePattern == 2) {
            // CIRCLE: projectiles in all directions
            int count = cfg_.projectiles.enemy.circleCount;
            for (int i = 0; i < count; ++i) {
                float angle = (2.0f * 3.14159f * i) / static_cast<float>(count);
                float circleSpeed = projSpeed * cfg_.projectiles.enemy.circleSpeedFactor;
                spawnSingleMissile(enemy, std::cos(angle) * circleSpeed, std::sin(angle) * circleSpeed, gs);
            }
        } else if (enemy.firePattern == 3) {
            // SPREAD: 3 projectiles in a fan
            for (int i = -1; i <= 1; ++i) {
                float angle = i * cfg_.projectiles.enemy.spreadAngle;
                float dx = -projSpeed * std::cos(angle);
                float dy = -projSpeed * std::sin(angle);
                spawnSingleMissile(enemy, dx, dy, gs);
            }
        }
    }
    
    void spawnSingleMissile(const ServerEntity& enemy, float vx, float vy, RoomGameState& gs) {
        ServerEntity missile;
        missile.id = nextEntityId_++;
        missile.type = EntityType::ENTITY_MONSTER_MISSILE;
        missile.x = enemy.x + cfg_.projectiles.enemy.spawnOffsetX;
        missile.y = enemy.y;
        missile.vx = vx;
        missile.vy = vy;
        missile.hp = 1;
        missile.playerId = 0;
        missile.playerLine = 0;
        missile.width = 26.0f;   // 13*2.0 scale
        missile.height = 16.0f;  // 8*2.0 scale
        
        gs.entities[missile.id] = missile;
        broadcastEntitySpawn(missile, gs.roomId);
    }
    
    const ServerEntity* findNearestPlayer(const ServerEntity& from, const RoomGameState& gs) {
        float nearestDist = 999999.0f;
        const ServerEntity* nearest = nullptr;
        for (const auto& [id, e] : gs.entities) {
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

    void spawnExplosion(float x, float y, RoomGameState& gs) {
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
        explosion.lifetime = cfg_.explosions.lifetime; // Explosions disappear after configured time
        
        gs.entities[explosion.id] = explosion;
        broadcastEntitySpawn(explosion, gs.roomId);
        
        std::cout << "[GameServer] Created explosion " << explosion.id << " at (" << x << ", " << y << ") with lifetime " << explosion.lifetime << "s" << std::endl;
    }

    void sendWorldSnapshot() {
        // Send one snapshot per room, using each room's own entities
        for (auto& [roomId, gs] : roomStates_) {
            auto room = server_.getRoomManager().getRoom(roomId);
            if (!room || room->state != RoomState::PLAYING) continue;
            
            std::vector<const ServerEntity*> snapshotEntities;
            
            // Add ALL entities from this room's game state
            for (const auto& [id, entity] : gs.entities) {
                snapshotEntities.push_back(&entity);
            }
            
            // Build packet
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
                state.hp = static_cast<uint16_t>(std::min(entity->hp, (int32_t)65535));
                state.playerLine = entity->playerLine;
                state.playerId = entity->playerId;
                state.chargeLevel = entity->chargeLevel;
                state.enemyType = entity->enemyType;
                state.score = entity->score;
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
    }

    void broadcastEntitySpawn(const ServerEntity& entity, uint32_t roomId) {
        EntityState state;
        state.id = entity.id;
        state.type = entity.type;
        state.x = entity.x;
        state.y = entity.y;
        state.vx = entity.vx;
        state.vy = entity.vy;
        state.hp = static_cast<uint16_t>(std::min(entity.hp, (int32_t)65535));
        state.playerLine = entity.playerLine;
        state.playerId = entity.playerId;
        state.chargeLevel = entity.chargeLevel;
        state.enemyType = entity.enemyType;
        state.projectileType = entity.projectileType;
        
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ENTITY_SPAWN));
        packet.header.timestamp = getCurrentTimestamp();
        packet.setPayload(state.serialize());
        
        broadcastToRoom(roomId, packet);
    }

    void broadcastEntityDestroy(uint32_t entityId, uint32_t roomId) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ENTITY_DESTROY));
        packet.header.timestamp = getCurrentTimestamp();
        
        std::vector<char> payload(sizeof(uint32_t));
        std::memcpy(payload.data(), &entityId, sizeof(uint32_t));
        packet.setPayload(payload);
        
        broadcastToRoom(roomId, packet);
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
        
        // Clean up player's game entities from room state
        auto gsIt = roomStates_.find(roomId);
        if (gsIt != roomStates_.end()) {
            RoomGameState& gs = gsIt->second;
            auto playerIt = gs.playerEntities.find(playerId);
            if (playerIt != gs.playerEntities.end()) {
                uint32_t entityId = playerIt->second;
                auto entityIt = gs.entities.find(entityId);
                if (entityIt != gs.entities.end()) {
                    spawnExplosion(entityIt->second.x, entityIt->second.y, gs);
                    gs.entities.erase(entityIt);
                    broadcastEntityDestroy(entityId, roomId);
                }
                gs.playerEntities.erase(playerIt);
                gs.playerPrevFire.erase(playerId);
                gs.playerLastCharge.erase(playerId);
            }
            
            // If room is now empty of players, clean up room state
            auto room = server_.getRoomManager().getRoom(roomId);
            if (!room || room->playerIds.empty()) {
                roomStates_.erase(gsIt);
                std::cout << "[GameServer] Cleaned up empty room state for room " << roomId << std::endl;
            }
        }
        
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
        
        // Check minimum player count
        if (room->playerIds.size() < static_cast<size_t>(cfg_.server.minPlayersToStart)) {
            std::cerr << "[GameServer] Cannot start game: only " << room->playerIds.size() 
                      << " player(s) in room (need at least " << cfg_.server.minPlayersToStart << ")" << std::endl;
            return;
        }
        
        // Change room state to PLAYING
        room->state = RoomState::PLAYING;
        
        std::cout << "[GameServer] ========== GAME STARTING in room " << session->roomId 
                  << " ==========" << std::endl;
        std::cout << "[GameServer] Creating player entities for " << room->playerIds.size() 
                  << " players..." << std::endl;
        
        // Create a new RoomGameState for this room
        RoomGameState& gs = roomStates_[session->roomId];
        gs.roomId = session->roomId;
        
        // Create player entities for all players in the room
        int playerIndex = 0;
        for (uint8_t playerId : room->playerIds) {
            // Map player to room for input routing
            playerToRoom_[playerId] = session->roomId;
            
            // Create player entity
            ServerEntity player;
            player.id = nextEntityId_++;
            player.type = EntityType::ENTITY_PLAYER;
            player.x = cfg_.player.spawnX;
            player.y = cfg_.player.spawnYStart + (playerIndex * cfg_.player.spawnYOffset);
            player.vx = 0.0f;
            player.vy = 0.0f;
            player.hp = cfg_.player.maxHealth;
            player.playerId = playerId;
            player.playerLine = playerIndex % cfg_.server.maxPlayerShips;
            player.width = 99.0f;   // 33*3.0 scale
            player.height = 51.0f;  // 17*3.0 scale
            
            gs.entities[player.id] = player;
            gs.playerEntities[playerId] = player.id;
            
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
    ServerConfig::Config cfg_;
    std::unordered_map<uint32_t, RoomGameState> roomStates_; // roomId -> per-room game state
    std::unordered_map<asio::ip::udp::endpoint, uint8_t> endpointToPlayerId_; // endpoint -> playerId
    std::unordered_map<uint8_t, uint32_t> playerToRoom_;  // playerId -> roomId (for routing input)
    uint32_t nextEntityId_;  // Global counter to ensure unique entity IDs across all rooms
    uint8_t nextPlayerId_ = 1;
    bool gameRunning_;
    std::mt19937 rng_;
    std::uniform_int_distribution<> dist_;
};

int main() {
    std::cout << "R-Type Server Starting..." << std::endl;

    try {
        // Load config first to get port
        ServerConfig::Config tempCfg;
        ServerConfig::loadFromLua(tempCfg, "assets/scripts/config/server_config.lua");
        
        GameServer server(static_cast<short>(tempCfg.server.port));
        server.start();
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
