#include "SimpleProtocol.hpp"
#include <network/NetworkServer.hpp>
#include <engine/Clock.hpp>
#include <iostream>
#include <random>
#include <cstring>
#include <thread>
#include <chrono>

using namespace FlappyBird;

// ============================================================================
// GAME STATE
// ============================================================================

enum class GameState {
    WAITING_FOR_PLAYERS,  // 0-1 player connected
    COUNTDOWN,            // 2 players, countdown in progress
    PLAYING,              // Game active
    GAME_OVER             // Game ended
};

struct Player {
    uint8_t id;                        // 1 or 2
    asio::ip::udp::endpoint endpoint;  // Network address
    float y;                           // Vertical position (0-720)
    float vy;                          // Vertical velocity
    bool isAlive;                      // Alive or dead
    uint16_t score;                    // Points scored
    bool isConnected;                  // Connection status
    
    Player() : id(0), y(360), vy(0), isAlive(true), score(0), isConnected(false) {}
};

struct Pipe {
    uint16_t id;       // Unique ID
    float x;           // Horizontal position
    float gapY;        // Gap vertical position
    float gapHeight;   // Gap height
    
    Pipe(uint16_t _id, float _x, float _gapY, float _gapHeight)
        : id(_id), x(_x), gapY(_gapY), gapHeight(_gapHeight) {}
};

// ============================================================================
// SIMPLE FLAPPY SERVER
// ============================================================================

class SimpleFlappyServer {
public:
    SimpleFlappyServer(short port) 
        : server_(port)
        , playerCount_(0)
        , state_(GameState::WAITING_FOR_PLAYERS)
        , nextPipeId_(1000)
        , pipeSpawnTimer_(0)
        , countdownTimer_(0)
        , countdownValue_(3)
    {
        std::random_device rd;
        rng_.seed(rd());
    }

    void start() {
        server_.start();
        std::cout << "========================================\n";
        std::cout << "  🐦 FLAPPY BIRD BATTLE ROYALE 🐦\n";
        std::cout << "     SIMPLE SERVER (2 PLAYERS)\n";
        std::cout << "========================================\n\n";
        std::cout << "✅ Server listening on port 8888\n";
        std::cout << "⏳ Waiting for players to connect...\n\n";
    }

    void run() {
        eng::engine::Clock clock;
        
        while (true) {
            float dt = clock.restart();
            
            // Process network packets
            server_.process();
            processPackets();
            
            // Update game logic based on state
            switch (state_) {
                case GameState::WAITING_FOR_PLAYERS:
                    // Just wait for connections
                    break;
                    
                case GameState::COUNTDOWN:
                    updateCountdown(dt);
                    break;
                    
                case GameState::PLAYING:
                    updatePhysics(dt);
                    updatePipes(dt);
                    checkCollisions();
                    
                    // Spawn pipes periodically
                    pipeSpawnTimer_ += dt;
                    if (pipeSpawnTimer_ >= PIPE_SPAWN_INTERVAL) {
                        pipeSpawnTimer_ = 0;
                        spawnPipe();
                    }
                    
                    // Broadcast game state at 30 Hz
                    static float stateTimer = 0;
                    stateTimer += dt;
                    if (stateTimer >= 1.0f / 30.0f) {
                        stateTimer = 0;
                        broadcastGameState();
                    }
                    break;
                    
                case GameState::GAME_OVER:
                    // Wait for reset
                    break;
            }
            
            // Check for client timeouts
            server_.checkTimeouts();
            
            // 60 Hz tick rate
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

private:
    // Physics constants (must match client!)
    static constexpr float GRAVITY = 980.0f;
    static constexpr float FLAP_STRENGTH = 350.0f;
    static constexpr float TERMINAL_VELOCITY = 600.0f;
    static constexpr float PIPE_SPEED = 200.0f;
    static constexpr float PIPE_SPAWN_INTERVAL = 2.0f;
    static constexpr float PIPE_GAP_HEIGHT = 180.0f;
    static constexpr float SCREEN_WIDTH = 1280.0f;
    static constexpr float SCREEN_HEIGHT = 720.0f;
    static constexpr float BIRD_SIZE = 34.0f;
    static constexpr float BIRD_X = 100.0f;  // Fixed X position for birds
    static constexpr float PIPE_WIDTH = 80.0f;

    NetworkServer server_;
    Player players_[2];
    int playerCount_;
    GameState state_;
    
    std::vector<Pipe> pipes_;
    uint16_t nextPipeId_;
    float pipeSpawnTimer_;
    
    float countdownTimer_;
    int countdownValue_;
    
    std::mt19937 rng_;

    // ========================================================================
    // PACKET PROCESSING
    // ========================================================================
    
    void processPackets() {
        while (server_.hasReceivedPackets()) {
            auto [packet, sender] = server_.getNextReceivedPacket();
            auto type = static_cast<PacketType>(packet.header.type);
            
            switch (type) {
                case PacketType::CLIENT_HELLO:
                    handleClientHello(sender);
                    break;
                    
                case PacketType::CLIENT_PING:
                    // Keep-alive, server automatically tracks this
                    break;
                    
                case PacketType::PLAYER_INPUT:
                    handlePlayerInput(packet, sender);
                    break;
                    
                default:
                    // Ignore unknown packets
                    break;
            }
        }
    }
    
    void handleClientHello(asio::ip::udp::endpoint sender) {
        // Check if player already connected (reconnection)
        for (int i = 0; i < 2; i++) {
            if (players_[i].isConnected && players_[i].endpoint == sender) {
                std::cout << "🔄 Player " << (int)players_[i].id << " reconnected\n";
                return;
            }
        }
        
        // Check if server is full
        if (playerCount_ >= 2) {
            std::cout << "❌ Server full, rejecting connection from " << sender << "\n";
            return;
        }
        
        // Add new player
        uint8_t playerId = (playerCount_ == 0) ? 1 : 2;
        int idx = playerId - 1;
        
        players_[idx].id = playerId;
        players_[idx].endpoint = sender;
        players_[idx].y = SCREEN_HEIGHT / 2;
        players_[idx].vy = 0;
        players_[idx].isAlive = true;
        players_[idx].score = 0;
        players_[idx].isConnected = true;
        
        playerCount_++;
        
        // Send WELCOME packet with player ID
        NetworkPacket welcome(static_cast<uint16_t>(PacketType::SERVER_WELCOME));
        welcome.payload.push_back(playerId);
        sendTo(welcome, sender);
        
        std::cout << "✅ Player " << (int)playerId << " joined from " << sender << "\n";
        std::cout << "📊 Players: " << playerCount_ << "/2\n";
        
        // If 2 players connected, start countdown!
        if (playerCount_ == 2) {
            std::cout << "\n🎮 Both players connected! Starting countdown...\n";
            startCountdown();
        } else {
            std::cout << "⏳ Waiting for second player...\n\n";
        }
    }
    
    void handlePlayerInput(const NetworkPacket& packet, asio::ip::udp::endpoint sender) {
        if (state_ != GameState::PLAYING) return;
        
        // Find player by endpoint
        int playerIdx = -1;
        for (int i = 0; i < 2; i++) {
            if (players_[i].isConnected && players_[i].endpoint == sender) {
                playerIdx = i;
                break;
            }
        }
        
        if (playerIdx == -1 || !players_[playerIdx].isAlive) return;
        
        // Apply flap (upward velocity)
        players_[playerIdx].vy = -FLAP_STRENGTH;
        std::cout << "🐦 Player " << (int)players_[playerIdx].id << " flapped!\n";
    }
    
    // ========================================================================
    // GAME LOGIC
    // ========================================================================
    
    void startCountdown() {
        state_ = GameState::COUNTDOWN;
        countdownTimer_ = 0;
        countdownValue_ = 3;
        
        // Broadcast countdown start
        NetworkPacket countdown(static_cast<uint16_t>(PacketType::START_COUNTDOWN));
        countdown.payload.push_back(3);
        broadcastPacket(countdown);
        
        std::cout << "⏱️  Countdown: 3...\n";
    }
    
    void updateCountdown(float dt) {
        countdownTimer_ += dt;
        
        if (countdownTimer_ >= 1.0f) {
            countdownTimer_ = 0;
            countdownValue_--;
            
            if (countdownValue_ > 0) {
                // Send next countdown number
                NetworkPacket countdown(static_cast<uint16_t>(PacketType::START_COUNTDOWN));
                countdown.payload.push_back(static_cast<uint8_t>(countdownValue_));
                broadcastPacket(countdown);
                
                std::cout << "⏱️  Countdown: " << countdownValue_ << "...\n";
            } else {
                // Countdown finished, start game!
                std::cout << "⏱️  Countdown: GO!\n\n";
                startGame();
            }
        }
    }
    
    void startGame() {
        state_ = GameState::PLAYING;
        
        // Reset player states
        for (int i = 0; i < 2; i++) {
            players_[i].y = SCREEN_HEIGHT / 2;
            players_[i].vy = 0;
            players_[i].isAlive = true;
            players_[i].score = 0;
        }
        
        // Clear pipes
        pipes_.clear();
        pipeSpawnTimer_ = 0;
        
        // Broadcast GAME_START
        NetworkPacket gameStart(static_cast<uint16_t>(PacketType::GAME_START));
        broadcastPacket(gameStart);
        
        std::cout << "🎮 Game started!\n";
        std::cout << "================================\n\n";
    }
    
    void updatePhysics(float dt) {
        for (int i = 0; i < 2; i++) {
            if (!players_[i].isAlive) continue;
            
            // Apply gravity
            players_[i].vy += GRAVITY * dt;
            if (players_[i].vy > TERMINAL_VELOCITY) {
                players_[i].vy = TERMINAL_VELOCITY;
            }
            
            // Update position
            players_[i].y += players_[i].vy * dt;
            
            // Check bounds (ceiling/floor death)
            if (players_[i].y < 0 || players_[i].y > SCREEN_HEIGHT - BIRD_SIZE) {
                killPlayer(i);
            }
        }
    }
    
    void updatePipes(float dt) {
        // Move pipes left
        for (auto& pipe : pipes_) {
            pipe.x -= PIPE_SPEED * dt;
        }
        
        // Remove off-screen pipes
        pipes_.erase(
            std::remove_if(pipes_.begin(), pipes_.end(),
                [](const Pipe& p) { return p.x < -200; }),
            pipes_.end()
        );
    }
    
    void spawnPipe() {
        // Random gap position
        std::uniform_int_distribution<int> dist(100, static_cast<int>(SCREEN_HEIGHT - PIPE_GAP_HEIGHT - 100));
        float gapY = static_cast<float>(dist(rng_));
        
        // Create pipe
        uint16_t pipeId = nextPipeId_++;
        pipes_.emplace_back(pipeId, SCREEN_WIDTH + PIPE_WIDTH, gapY, PIPE_GAP_HEIGHT);
        
        // Broadcast SPAWN_PIPE
        NetworkPacket spawnPacket(static_cast<uint16_t>(PacketType::SPAWN_PIPE));
        
        // pipeId (2 bytes)
        spawnPacket.payload.push_back((pipeId >> 8) & 0xFF);
        spawnPacket.payload.push_back(pipeId & 0xFF);
        
        // x (2 bytes)
        uint16_t x = static_cast<uint16_t>(pipes_.back().x);
        spawnPacket.payload.push_back((x >> 8) & 0xFF);
        spawnPacket.payload.push_back(x & 0xFF);
        
        // gapY (2 bytes)
        uint16_t gy = static_cast<uint16_t>(gapY);
        spawnPacket.payload.push_back((gy >> 8) & 0xFF);
        spawnPacket.payload.push_back(gy & 0xFF);
        
        // gapHeight (2 bytes)
        uint16_t gh = static_cast<uint16_t>(PIPE_GAP_HEIGHT);
        spawnPacket.payload.push_back((gh >> 8) & 0xFF);
        spawnPacket.payload.push_back(gh & 0xFF);
        
        broadcastPacket(spawnPacket);
        
        std::cout << "🚧 Spawned pipe " << pipeId << " at gapY=" << gapY << "\n";
    }
    
    void checkCollisions() {
        for (int i = 0; i < 2; i++) {
            if (!players_[i].isAlive) continue;
            
            float birdLeft = BIRD_X;
            float birdRight = BIRD_X + BIRD_SIZE;
            float birdTop = players_[i].y;
            float birdBottom = players_[i].y + BIRD_SIZE;
            
            for (auto& pipe : pipes_) {
                float pipeLeft = pipe.x;
                float pipeRight = pipe.x + PIPE_WIDTH;
                
                // Check if bird is in pipe's X range
                if (birdRight > pipeLeft && birdLeft < pipeRight) {
                    // Check top pipe collision
                    if (birdTop < pipe.gapY) {
                        std::cout << "💥 Player " << (int)players_[i].id << " hit top pipe!\n";
                        killPlayer(i);
                        return;
                    }
                    
                    // Check bottom pipe collision
                    if (birdBottom > pipe.gapY + pipe.gapHeight) {
                        std::cout << "💥 Player " << (int)players_[i].id << " hit bottom pipe!\n";
                        killPlayer(i);
                        return;
                    }
                }
                
                // Check if bird passed pipe (scoring)
                if (birdLeft > pipeRight && pipe.x < BIRD_X + 10 && pipe.x > BIRD_X - 10) {
                    players_[i].score++;
                    std::cout << "⭐ Player " << (int)players_[i].id << " scored! Total: " << players_[i].score << "\n";
                }
            }
        }
    }
    
    void killPlayer(int playerIdx) {
        if (!players_[playerIdx].isAlive) return;
        
        players_[playerIdx].isAlive = false;
        
        // Broadcast PLAYER_DIED
        NetworkPacket died(static_cast<uint16_t>(PacketType::PLAYER_DIED));
        died.payload.push_back(players_[playerIdx].id);
        broadcastPacket(died);
        
        std::cout << "☠️  Player " << (int)players_[playerIdx].id << " died! Final score: " << players_[playerIdx].score << "\n";
        
        // Check game over
        int aliveCount = 0;
        uint8_t winnerId = 0;
        
        for (int i = 0; i < 2; i++) {
            if (players_[i].isAlive) {
                aliveCount++;
                winnerId = players_[i].id;
            }
        }
        
        if (aliveCount <= 1) {
            endGame(winnerId);
        }
    }
    
    void endGame(uint8_t winnerId) {
        state_ = GameState::GAME_OVER;
        
        std::cout << "\n================================\n";
        std::cout << "🏆 GAME OVER!\n";
        std::cout << "   Winner: Player " << (int)winnerId << "\n";
        std::cout << "================================\n\n";
        
        // Broadcast GAME_OVER
        NetworkPacket gameOver(static_cast<uint16_t>(PacketType::GAME_OVER));
        gameOver.payload.push_back(winnerId);
        broadcastPacket(gameOver);
        
        // Reset after 5 seconds
        std::cout << "⏳ Server will reset in 5 seconds...\n\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // Reset state
        playerCount_ = 0;
        for (int i = 0; i < 2; i++) {
            players_[i] = Player();
        }
        pipes_.clear();
        state_ = GameState::WAITING_FOR_PLAYERS;
        
        std::cout << "🔄 Server reset! Waiting for new players...\n\n";
    }
    
    // ========================================================================
    // NETWORK
    // ========================================================================
    
    void broadcastGameState() {
        NetworkPacket state(static_cast<uint16_t>(PacketType::GAME_STATE));
        
        // Player 1 (7 bytes)
        uint16_t p1y = static_cast<uint16_t>(players_[0].y);
        state.payload.push_back((p1y >> 8) & 0xFF);
        state.payload.push_back(p1y & 0xFF);
        
        int16_t p1vy = static_cast<int16_t>(players_[0].vy);
        state.payload.push_back((p1vy >> 8) & 0xFF);
        state.payload.push_back(p1vy & 0xFF);
        
        state.payload.push_back(players_[0].isAlive ? 1 : 0);
        
        state.payload.push_back((players_[0].score >> 8) & 0xFF);
        state.payload.push_back(players_[0].score & 0xFF);
        
        // Player 2 (7 bytes)
        uint16_t p2y = static_cast<uint16_t>(players_[1].y);
        state.payload.push_back((p2y >> 8) & 0xFF);
        state.payload.push_back(p2y & 0xFF);
        
        int16_t p2vy = static_cast<int16_t>(players_[1].vy);
        state.payload.push_back((p2vy >> 8) & 0xFF);
        state.payload.push_back(p2vy & 0xFF);
        
        state.payload.push_back(players_[1].isAlive ? 1 : 0);
        
        state.payload.push_back((players_[1].score >> 8) & 0xFF);
        state.payload.push_back(players_[1].score & 0xFF);
        
        broadcastPacket(state);
    }
    
    void broadcastPacket(const NetworkPacket& packet) {
        for (int i = 0; i < 2; i++) {
            if (players_[i].isConnected) {
                server_.sendTo(packet, players_[i].endpoint);
            }
        }
    }
    
    void sendTo(const NetworkPacket& packet, asio::ip::udp::endpoint endpoint) {
        server_.sendTo(packet, endpoint);
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    short port = 8888;
    
    if (argc > 1) {
        port = static_cast<short>(std::atoi(argv[1]));
    }
    
    try {
        SimpleFlappyServer server(port);
        server.start();
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "❌ Server error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
