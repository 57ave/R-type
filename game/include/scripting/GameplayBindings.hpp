#ifndef RTYPE_GAME_SCRIPTING_GAMEPLAY_BINDINGS_HPP
#define RTYPE_GAME_SCRIPTING_GAMEPLAY_BINDINGS_HPP

#include <ecs/Coordinator.hpp>
#include <functional>
#include <map>
#include <scripting/LuaState.hpp>
#include <sol/sol.hpp>
#include <string>
#include <vector>

/**
 * @file GameplayBindings.hpp
 * @brief Lua bindings for data-driven gameplay systems
 *
 * This provides the interface between Lua configuration and C++ gameplay.
 * All game entities, weapons, enemies, bosses are configured in Lua.
 */

namespace RType {
namespace Scripting {

/**
 * @brief Data structures for Lua-configured gameplay elements
 */

// Spawn request from Lua
struct LuaSpawnRequest {
    std::string entityType;  // "enemy", "boss", "powerup", etc.
    std::string subType;     // "basic", "zigzag", "stage1_boss", etc.
    float x = 1920.0f;
    float y = 540.0f;
    std::string pattern = "straight";
    std::map<std::string, float> customParams;
};

// Weapon configuration from Lua
struct LuaWeaponConfig {
    std::string name;
    float fireRate = 0.2f;
    float projectileSpeed = 1000.0f;
    int damage = 10;
    int projectileCount = 1;
    float spreadAngle = 0.0f;
    bool canCharge = false;
    float maxChargeTime = 1.0f;
    bool piercing = false;
    bool homing = false;
    float homingStrength = 0.0f;
};

// Enemy configuration from Lua
struct LuaEnemyConfig {
    std::string name;
    std::string type;
    int health = 10;
    int damage = 10;
    float speed = 200.0f;
    int scoreValue = 100;
    std::string movementPattern = "straight";
    float amplitude = 0.0f;
    float frequency = 0.0f;
    std::string weaponType;
    float shootInterval = 0.0f;
    float dropChance = 0.0f;
    std::vector<std::string> dropTable;

    // Sprite info
    std::string texture;
    int frameWidth = 32;
    int frameHeight = 32;
    float scale = 2.0f;
    int frameCount = 1;
    float frameTime = 0.1f;
};

// Boss configuration from Lua
struct LuaBossConfig {
    std::string name;
    std::string type;
    int health = 500;
    int maxHealth = 500;
    int scoreValue = 10000;

    // Entry
    float entryStartX = 2200.0f;
    float entryTargetX = 1450.0f;
    float entryDuration = 4.0f;

    // Movement
    std::string movementPattern = "hover";
    float amplitude = 80.0f;
    float frequency = 0.5f;

    // Phases
    int phaseCount = 3;
    std::vector<float> phaseThresholds;
    std::vector<std::string> phaseAttacks;

    // Sprite
    std::string texture;
    int frameWidth = 160;
    int frameHeight = 128;
    float scale = 2.0f;
};

// Wave info from Lua
struct LuaWaveInfo {
    std::string name;
    int index = 0;
    bool isBossWave = false;
    std::string bossType;
    float startTime = 0.0f;
    float duration = 30.0f;
};

/**
 * @brief GameplayBindings - Bridge between Lua config and C++ gameplay
 */
class GameplayBindings {
public:
    // Callbacks for entity creation (set by game code)
    using EnemySpawnCallback = std::function<ECS::Entity(const LuaEnemyConfig&, float x, float y,
                                                         const std::string& pattern)>;
    using BossSpawnCallback = std::function<ECS::Entity(const LuaBossConfig&)>;
    using PowerUpSpawnCallback =
        std::function<ECS::Entity(const std::string& type, float x, float y)>;
    using ProjectileSpawnCallback = std::function<ECS::Entity(const LuaWeaponConfig&, float x,
                                                              float y, float angle, bool isPlayer)>;

    /**
     * @brief Register all gameplay bindings
     */
    static void RegisterAll(sol::state& lua, ECS::Coordinator* coordinator);

    /**
     * @brief Register spawn callbacks
     */
    static void SetEnemySpawnCallback(EnemySpawnCallback cb) { s_enemySpawnCb = cb; }
    static void SetBossSpawnCallback(BossSpawnCallback cb) { s_bossSpawnCb = cb; }
    static void SetPowerUpSpawnCallback(PowerUpSpawnCallback cb) { s_powerUpSpawnCb = cb; }
    static void SetProjectileSpawnCallback(ProjectileSpawnCallback cb) { s_projectileSpawnCb = cb; }

    /**
     * @brief Load and execute the master config
     */
    static bool LoadMasterConfig(sol::state& lua, const std::string& basePath);

    /**
     * @brief Get configuration from Lua
     */
    static LuaEnemyConfig GetEnemyConfig(sol::state& lua, const std::string& enemyType);
    static LuaBossConfig GetBossConfig(sol::state& lua, const std::string& bossType);
    static LuaWeaponConfig GetWeaponConfig(sol::state& lua, const std::string& weaponName,
                                           int level = 1);
    static LuaWaveInfo GetActiveWave(sol::state& lua, int stageNumber, float stageTime);

    /**
     * @brief Spawn system interface
     */
    static void StartStage(sol::state& lua, int stageNumber);
    static std::vector<LuaSpawnRequest> UpdateSpawns(sol::state& lua, float deltaTime);
    static std::string CheckBossSpawn(sol::state& lua);

    /**
     * @brief Difficulty system
     */
    static void SetDifficulty(sol::state& lua, const std::string& difficulty);
    static std::string GetDifficulty(sol::state& lua);

private:
    static EnemySpawnCallback s_enemySpawnCb;
    static BossSpawnCallback s_bossSpawnCb;
    static PowerUpSpawnCallback s_powerUpSpawnCb;
    static ProjectileSpawnCallback s_projectileSpawnCb;

    // Register individual binding categories
    static void RegisterSpawnFunctions(sol::state& lua, ECS::Coordinator* coordinator);
    static void RegisterConfigAccessors(sol::state& lua);
    static void RegisterGameState(sol::state& lua);
};

}  // namespace Scripting
}  // namespace RType

#endif  // RTYPE_GAME_SCRIPTING_GAMEPLAY_BINDINGS_HPP
