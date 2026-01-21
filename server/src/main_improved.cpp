#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <cstdint>
#include <algorithm>
#include <cmath>
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
    uint8_t hp;
    uint8_t playerId; // For player entities
    uint8_t playerLine; // For player ship color (spritesheet line)
    float fireTimer = 0.0f; // For rate limiting
    float lifetime = -1.0f; // For temporary entities like explosions (-1 = permanent)
    
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
        eng::engine::Clock updateClock;
        eng::engine::Clock snapshotClock;
        
        const float fixedDeltaTime = 1.0f / 60.0f; // 60 FPS simulation
        float enemySpawnTimer = 0.0f;
        const float enemySpawnInterval = 2.0f;
        
        const float snapshotRate = 1.0f / 30.0f; // 30 snapshots per second
        float accumulatedTime = 0.0f;

        // Stats: print every second
        // statsClock_ is a member Clock (initialized default)
        const float statsInterval = 1.0f;

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
                    enemySpawnTimer += fixedDeltaTime;
                    if (enemySpawnTimer >= enemySpawnInterval) {
                        enemySpawnTimer = 0.0f;
                        spawnEnemy();
                    }
                }
                
                // Send world snapshot at reduced rate (30Hz)
                if (snapshotClock.getElapsedTime() >= snapshotRate) {
                    snapshotClock.restart();
                    sendWorldSnapshot();
                }
                
                // Check for timeouts
                server_.checkTimeouts();
            }
            
            // Stats printing every second
            if (statsClock_.getElapsedTime() >= statsInterval) {
                statsClock_.restart();
                uint64_t sent = bytesSentLastInterval_;
                uint64_t recv = bytesReceivedLastInterval_;
                bytesSentLastInterval_ = 0;
                bytesReceivedLastInterval_ = 0;
                std::cout << "[NetworkStats] Sent: " << sent << " B/s  Received: " << recv << " B/s"
                          << "  TotalSent: " << bytesSentTotal_ << " B  TotalRecv: " << bytesReceivedTotal_ << " B"
                          << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    void processPackets() {
        while (server_.hasReceivedPackets()) {
            auto [packet, sender] = server_.getNextReceivedPacket();

            // Count received bytes (header fields + payload)
            size_t headerBytes = sizeof(packet.header.type) + sizeof(packet.header.timestamp);
            size_t packetSize = headerBytes + packet.payload.size();
            bytesReceivedTotal_ += packetSize;
            bytesReceivedLastInterval_ += packetSize;

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
        
        // use wrapper to count bytes
        sendToCount(welcome, sender);
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

        // Fire (with rate limiting)
        if ((input.inputMask & (1 << 4)) && player.fireTimer <= 0.0f) {
            spawnPlayerMissile(player, input.chargeLevel);
            player.fireTimer = 0.2f; // 0.2 second cooldown
        }

        // Update last input time and mask for prediction
        lastPlayerInputTime_[input.playerId] = std::chrono::steady_clock::now();
        lastPlayerInputMask_[input.playerId] = input.inputMask;
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
            sendToCount(reply, sender);
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

    void predictPlayerMovement(ServerEntity& entity)
    {
        if (entity.type == EntityType::ENTITY_PLAYER) {
            uint8_t pid = entity.playerId;
            auto itTime = lastPlayerInputTime_.find(pid);
            if (itTime != lastPlayerInputTime_.end()) {
                auto now = std::chrono::steady_clock::now();
                auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - itTime->second).count();
                const int predictionWindowMs = 200; // predict for 200 ms after last input
                if (elapsedMs <= predictionWindowMs) {
                    uint8_t inputMask = lastPlayerInputMask_[pid];
                    const float speed = 500.0f;
                    entity.vx = 0.0f;
                    entity.vy = 0.0f;
                    if (inputMask & (1 << 0)) entity.vy = -speed; // Up
                    if (inputMask & (1 << 1)) entity.vy = speed;  // Down
                    if (inputMask & (1 << 2)) entity.vx = -speed; // Left
                    if (inputMask & (1 << 3)) entity.vx = speed;  // Right
                    // We intentionally DO NOT spawn missiles here — only spawn on receipt of input packet earlier
                }
            }
        }
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
        explosion.lifetime = 0.5f; // Explosions disappear after 0.5 seconds
        
        entities_[explosion.id] = explosion;
        broadcastEntitySpawn(explosion);
        
        std::cout << "[GameServer] Created explosion " << explosion.id << " at (" << x << ", " << y << ") with lifetime " << explosion.lifetime << "s" << std::endl;
    }

    void sendWorldSnapshot(bool forceFull = false) {
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
        
        // helper: comparison with small epsilon to avoid sending tiny float noise
        auto changed = [](const EntityState& a, const EntityState& b) {
            const float posEps = 0.05f; // 5cm tolerance
            const float velEps = 0.01f;
            if (a.type != b.type) return true;
            if (std::abs(a.x - b.x) > posEps) return true;
            if (std::abs(a.y - b.y) > posEps) return true;
            if (std::abs(a.vx - b.vx) > velEps) return true;
            if (std::abs(a.vy - b.vy) > velEps) return true;
            if (a.hp != b.hp) return true;
            if (a.playerLine != b.playerLine) return true;
            if (a.playerId != b.playerId) return true;
            if (a.chargeLevel != b.chargeLevel) return true;
            if (a.enemyType != b.enemyType) return true;
            if (a.projectileType != b.projectileType) return true;
            return false;
        };

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
                
                // Ajouter les autres entités (ennemis, projectiles, etc.)
                for (const auto& [id, entity] : entities_) {
                    if (entity.type == EntityType::ENTITY_EXPLOSION) continue;
                    if (entity.type == EntityType::ENTITY_PLAYER) continue;
                    snapshotEntities.push_back(&entity);
                }

                // Build a list of entities that are new or changed compared to last snapshot for this room
                SnapshotHeader header;
                header.entityCount = 0; // will be updated
                NetworkPacket packet(static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT));
                packet.header.timestamp = getCurrentTimestamp();

                auto headerData = header.serialize();
                // we'll reserve payload but insert header later after we know count
                std::vector<char> payload;

                // reference to the per-room cache
                auto& cache = lastSnapshotPerRoom_[roomId];

                // mark seen ids to cleanup stale cache entries later
                std::unordered_set<uint32_t> seenIds;

                for (const auto* entity : snapshotEntities) {
                    EntityState state;
                    state.id = entity->id;
                    state.type = entity->type;
                    state.x = entity->x;
                    state.y = entity->y;
                    state.vx = entity->vx;
                    state.vy = entity->vy;
                    state.hp = entity->hp;
                    state.playerLine = entity->playerLine;
                    state.playerId = entity->playerId; // include server-side playerId mapping
                    state.chargeLevel = entity->chargeLevel;
                    state.enemyType = entity->enemyType;
                    state.projectileType = entity->projectileType;

                    seenIds.insert(state.id);

                    bool isNew = (cache.find(state.id) == cache.end());
                    bool isChanged = false;
                    if (!isNew) {
                        isChanged = changed(state, cache[state.id]);
                    }

                    if (forceFull || isNew || isChanged) {
                        auto stateData = state.serialize();
                        payload.insert(payload.end(), stateData.begin(), stateData.end());
                        header.entityCount++;
                        // update cache immediately so repeated comparisons in same frame are consistent
                        cache[state.id] = state;
                    } else {
                        // not sent; keep existing cache entry as-is
                    }
                }

                // Cleanup cache entries that are no longer present in current snapshotEntities
                std::vector<uint32_t> toErase;
                for (auto& [cachedId, _s] : cache) {
                    if (seenIds.find(cachedId) == seenIds.end()) {
                        toErase.push_back(cachedId);
                    }
                }
                for (uint32_t id : toErase) cache.erase(id);

                // If there are no changed/new entities and not forcing full, skip sending
                if (header.entityCount == 0 && !forceFull) {
                    continue;
                }

                // Prepend header with correct entityCount
                auto hdr = header.serialize();
                packet.payload.insert(packet.payload.end(), hdr.begin(), hdr.end());
                packet.payload.insert(packet.payload.end(), payload.begin(), payload.end());

                // Send to room
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
            header.entityCount = 0;
            NetworkPacket packet(static_cast<uint16_t>(GamePacketType::WORLD_SNAPSHOT));
            packet.header.timestamp = getCurrentTimestamp();

            std::vector<char> payload;

            auto& cache = lastSnapshotGlobal_;
            std::unordered_set<uint32_t> seenIds;

            for (const auto* entity : snapshotEntities) {
                EntityState state;
                state.id = entity->id;
                state.type = entity->type;
                state.x = entity->x;
                state.y = entity->y;
                state.vx = entity->vx;
                state.vy = entity->vy;
                state.hp = entity->hp;
                state.playerLine = entity->playerLine;
                state.playerId = entity->playerId;
                state.chargeLevel = entity->chargeLevel;
                state.enemyType = entity->enemyType;
                state.projectileType = entity->projectileType;

                seenIds.insert(state.id);

                bool isNew = (cache.find(state.id) == cache.end());
                bool isChanged = false;
                if (!isNew) {
                    isChanged = changed(state, cache[state.id]);
                }

                if (forceFull || isNew || isChanged) {
                    auto stateData = state.serialize();
                    payload.insert(payload.end(), stateData.begin(), stateData.end());
                    header.entityCount++;
                    cache[state.id] = state;
                }
            }

            // Cleanup
            std::vector<uint32_t> toErase;
            for (auto& [cachedId, _s] : cache) {
                if (seenIds.find(cachedId) == seenIds.end()) {
                    toErase.push_back(cachedId);
                }
            }
            for (uint32_t id : toErase) cache.erase(id);

            if (header.entityCount == 0 && !forceFull) {
                return;
            }

            auto hdr = header.serialize();
            packet.payload.insert(packet.payload.end(), hdr.begin(), hdr.end());
            packet.payload.insert(packet.payload.end(), payload.begin(), payload.end());

            // use broadcast wrapper to count bytes for all recipients
            broadcastCount(packet);
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
        state.hp = entity.hp;
        state.playerLine = entity.playerLine;
        state.playerId = entity.playerId;
        state.chargeLevel = entity.chargeLevel;
        state.enemyType = entity.enemyType;
        state.projectileType = entity.projectileType;
        
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ENTITY_SPAWN));
        packet.header.timestamp = getCurrentTimestamp();
        packet.setPayload(state.serialize());

        // use broadcast wrapper
        broadcastCount(packet);
    }

    void broadcastEntityDestroy(uint32_t entityId) {
        NetworkPacket packet(static_cast<uint16_t>(GamePacketType::ENTITY_DESTROY));
        packet.header.timestamp = getCurrentTimestamp();

        std::vector<char> payload(sizeof(uint32_t));
        std::memcpy(payload.data(), &entityId, sizeof(uint32_t));
        packet.setPayload(payload);
        
        // use broadcast wrapper
        broadcastCount(packet);
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

        sendToCount(reply, sender);

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
            sendToCount(createdReply, sender);

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
            sendToCount(joinedReply, sender);

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
                sendToCount(reply, sender);

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
        sendWorldSnapshot(true); // true = force full snapshot

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
                // use per-recipient send counter
                sendToCount(packet, session.endpoint);
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
            info.isReady = false;  // TODO: ajouter système de ready
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

    // ------------------ Networking helpers & stats ------------------
    // Wrapper to send and count bytes for a single recipient
    void sendToCount(const NetworkPacket& packet, const asio::ip::udp::endpoint& endpoint) {
        // compute bytes: header fields + payload
        size_t headerBytes = sizeof(packet.header.type) + sizeof(packet.header.timestamp);
        size_t packetSize = headerBytes + packet.payload.size();

        server_.sendTo(packet, endpoint);
        bytesSentTotal_ += packetSize;
        bytesSentLastInterval_ += packetSize;
    }

    // Wrapper to broadcast to all active sessions and count bytes * recipients
    void broadcastCount(const NetworkPacket& packet) {
        size_t headerBytes = sizeof(packet.header.type) + sizeof(packet.header.timestamp);
        size_t packetSize = headerBytes + packet.payload.size();

        // get recipient count
        auto sessions = server_.getActiveSessions();
        size_t recipients = sessions.size();

        server_.broadcast(packet);

        bytesSentTotal_ += packetSize * recipients;
        bytesSentLastInterval_ += packetSize * recipients;
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
    // Track last input time and last input mask per player for client-side prediction on the server
    std::unordered_map<uint8_t, std::chrono::steady_clock::time_point> lastPlayerInputTime_;
    std::unordered_map<uint8_t, uint8_t> lastPlayerInputMask_;
    uint32_t nextEntityId_;
    uint8_t nextPlayerId_ = 1;
    bool gameRunning_;
    std::mt19937 rng_;
    std::uniform_int_distribution<> dist_;

    // ---------------- stats ----------------
    eng::engine::Clock statsClock_;
    uint64_t bytesSentTotal_ = 0;
    uint64_t bytesReceivedTotal_ = 0;
    uint64_t bytesSentLastInterval_ = 0;
    uint64_t bytesReceivedLastInterval_ = 0;

    // Cache for last sent snapshot per room (for delta updates)
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, EntityState>> lastSnapshotPerRoom_;
    // Global cache for last sent snapshot (classique mode)
    std::unordered_map<uint32_t, EntityState> lastSnapshotGlobal_;
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
