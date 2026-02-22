#pragma once

#include <string>
#include <vector>
#include <cstdint>

// ==========================================
// Server Configuration - loaded from Lua
// ==========================================

namespace ServerConfig {

    // --- Player ---
    struct PlayerConfig {
        float speed = 500.0f;
        int maxHealth = 100;
        float spawnX = 100.0f;
        float spawnYStart = 200.0f;
        float spawnYOffset = 200.0f;
        float boundaryMinX = 0.0f;
        float boundaryMinY = 0.0f;
        float boundaryMaxX = 1820.0f;
        float boundaryMaxY = 1030.0f;
    };

    // --- Enemies ---
    struct EnemyTypeConfig {
        uint8_t typeId;
        int health;
        float vx;
        float vy;
        uint8_t firePattern;
        float fireRate;
        int collisionDamage;
        int score;
        // Fighter-specific
        float zigzagInterval = 1.0f;
        float boundaryTop = 50.0f;
        float boundaryBottom = 1000.0f;
        // Kamikaze-specific
        float trackingSpeed = 500.0f;
    };

    struct EnemySpawnConfig {
        float spawnX = 1920.0f;
        float spawnYMin = 100.0f;
        int spawnYRange = 880;
        float fireTimerBase = 1.0f;
        int fireTimerRandomRange = 200;
    };

    // --- Bosses ---
    struct BossMovementConfig {
        float spawnX = 1920.0f;
        float spawnY = 400.0f;
        float stopX = 1500.0f;
        float bobSpeed = 1.5f;
        float bobAmplitude = 100.0f;
        float boundaryTop = 50.0f;
        float boundaryBottom = 900.0f;
        int score = 500;
        int collisionDamageToPlayer = 30;
        int collisionDamageFromPlayer = 20;
    };

    // --- Projectiles ---
    struct PlayerProjectileConfig {
        float normalSpeed = 800.0f;
        float chargedSpeed = 1500.0f;
        int baseDamage = 10;
        int chargeDamageMultiplier = 10;
        float fireCooldownNormal = 0.15f;
        float fireCooldownCharged = 0.3f;
        float spawnOffsetX = 50.0f;
        float spawnOffsetY = 10.0f;
    };

    struct EnemyProjectileConfig {
        float speedMultiplier = 1.5f;
        float minSpeed = 400.0f;
        int circleCount = 8;
        float circleSpeedFactor = 0.8f;
        float spreadAngle = 0.26f;
        float spawnOffsetX = -40.0f;
    };

    struct ProjectileConfig {
        PlayerProjectileConfig player;
        EnemyProjectileConfig enemy;
        int missileDamage = 10;
    };

    // --- Modules ---
    struct HomingConfig {
        float speed = 500.0f;
        float detectionRadius = 600.0f;
        float turnRate = 5.0f;
        uint8_t projectileType = 3;
    };

    struct SpreadConfig {
        std::vector<float> angles = {-0.2617f, 0.0f, 0.2617f};
        uint8_t projectileType = 4;
    };

    struct WaveConfig {
        float amplitude = 60.0f;
        float frequency = 4.0f;
        uint8_t projectileType = 5;
    };

    struct ModuleConfig {
        float fireCooldown = 0.2f;
        float baseSpeed = 800.0f;
        HomingConfig homing;
        SpreadConfig spread;
        WaveConfig wave;
        float spawnVx = -100.0f;
    };

    // --- Powerups ---
    struct OrangePowerupConfig {
        float bossDamageFraction = 0.25f;
    };

    struct BluePowerupConfig {
        float duration = 10.0f;
    };

    struct PowerupConfig {
        float spawnVx = -150.0f;
        float spawnX = 1920.0f;
        float spawnYMin = 100.0f;
        int spawnYRange = 880;
        OrangePowerupConfig orange;
        BluePowerupConfig blue;
    };

    // --- Explosions ---
    struct ExplosionConfig {
        float lifetime = 0.5f;
    };

    // --- Collisions ---
    struct CollisionConfig {
        float hitboxSize = 50.0f;
        float oobMargin = 100.0f;
        float screenWidth = 1920.0f;
        float screenHeight = 1080.0f;
    };

    // --- Level wave/boss ---
    struct WaveEnemyGroup {
        uint8_t type;
        int count;
        float interval;
    };

    struct WaveDefinition {
        float time;
        std::vector<WaveEnemyGroup> groups;
    };

    struct BossDefinition {
        uint8_t enemyType;
        uint16_t health;
        float speed;
        float fireRate;
        uint8_t firePattern;
        float spawnTime;
    };

    struct LevelDefinition {
        int id;
        std::string name;
        std::vector<uint8_t> enemyTypes;
        std::vector<uint8_t> moduleTypes;
        float enemyInterval;
        float powerupInterval;
        float moduleInterval;
        int maxEnemies;
        bool stopSpawningAtBoss;
        std::vector<WaveDefinition> waves;
        BossDefinition boss;
    };

    // --- Server ---
    struct ServerSettings {
        std::string serverIp = "127.0.0.1";
        int port = 12345;
        int tickRate = 60;
        int snapshotRate = 30;
        int minPlayersToStart = 2;
        int maxPlayerShips = 5;
    };

    // ==========================================
    // Main config aggregate
    // ==========================================
    struct Config {
        PlayerConfig player;
        EnemyTypeConfig bug;
        EnemyTypeConfig fighter;
        EnemyTypeConfig kamikaze;
        EnemySpawnConfig enemySpawn;
        BossMovementConfig bossMovement;
        ProjectileConfig projectiles;
        ModuleConfig modules;
        PowerupConfig powerups;
        ExplosionConfig explosions;
        CollisionConfig collisions;
        std::vector<LevelDefinition> levels; // index 0 = level 1
        ServerSettings server;
        int maxLevel = 3;
    };

    // Load configuration from Lua file
    // Returns true on success, false on failure (defaults are kept)
    bool loadFromLua(Config& config, const std::string& luaPath);

} // namespace ServerConfig
