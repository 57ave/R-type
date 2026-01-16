#include <scripting/GameplayBindings.hpp>
#include <iostream>
#include <fstream>

namespace RType {
namespace Scripting {

// Static callback storage
GameplayBindings::EnemySpawnCallback GameplayBindings::s_enemySpawnCb = nullptr;
GameplayBindings::BossSpawnCallback GameplayBindings::s_bossSpawnCb = nullptr;
GameplayBindings::PowerUpSpawnCallback GameplayBindings::s_powerUpSpawnCb = nullptr;
GameplayBindings::ProjectileSpawnCallback GameplayBindings::s_projectileSpawnCb = nullptr;

void GameplayBindings::RegisterAll(sol::state& lua, ECS::Coordinator* coordinator) {
    std::cout << "[GameplayBindings] Registering Lua gameplay bindings..." << std::endl;
    
    RegisterSpawnFunctions(lua, coordinator);
    RegisterConfigAccessors(lua);
    RegisterGameState(lua);
    
    std::cout << "[GameplayBindings] Gameplay bindings registered" << std::endl;
}

void GameplayBindings::RegisterSpawnFunctions(sol::state& lua, ECS::Coordinator* coordinator) {
    // Create Spawner table for Lua to call
    lua["Spawner"] = lua.create_table();
    
    // Spawn enemy from C++ callbacks
    lua["Spawner"]["SpawnEnemy"] = [](const std::string& enemyType, float x, float y, 
                                       sol::optional<std::string> pattern) -> int {
        if (!s_enemySpawnCb) {
            std::cerr << "[Spawner] No enemy spawn callback set!" << std::endl;
            return 0;
        }
        
        // Get config from Lua (will be implemented by game)
        // For now, create a basic config
        LuaEnemyConfig config;
        config.type = enemyType;
        
        std::string pat = pattern.value_or("straight");
        ECS::Entity entity = s_enemySpawnCb(config, x, y, pat);
        return static_cast<int>(entity);
    };
    
    // Spawn boss
    lua["Spawner"]["SpawnBoss"] = [](const std::string& bossType, float x, float y) -> int {
        if (!s_bossSpawnCb) {
            std::cerr << "[Spawner] No boss spawn callback set!" << std::endl;
            return 0;
        }
        
        LuaBossConfig config;
        config.type = bossType;
        
        ECS::Entity entity = s_bossSpawnCb(config);
        return static_cast<int>(entity);
    };
    
    // Spawn power-up
    lua["Spawner"]["SpawnPowerUp"] = [](const std::string& powerUpType, float x, float y) -> int {
        if (!s_powerUpSpawnCb) {
            std::cerr << "[Spawner] No power-up spawn callback set!" << std::endl;
            return 0;
        }
        
        ECS::Entity entity = s_powerUpSpawnCb(powerUpType, x, y);
        return static_cast<int>(entity);
    };
    
    // Spawn projectile
    lua["Spawner"]["SpawnProjectile"] = [](const std::string& weaponType, float x, float y, 
                                            float angle, bool isPlayer) -> int {
        if (!s_projectileSpawnCb) {
            std::cerr << "[Spawner] No projectile spawn callback set!" << std::endl;
            return 0;
        }
        
        LuaWeaponConfig config;
        config.name = weaponType;
        
        ECS::Entity entity = s_projectileSpawnCb(config, x, y, angle, isPlayer);
        return static_cast<int>(entity);
    };
}

void GameplayBindings::RegisterConfigAccessors(sol::state& lua) {
    // Create ConfigAccess table
    lua["ConfigAccess"] = lua.create_table();
    
    // These will call back into Lua's GameAPI
    lua["ConfigAccess"]["GetEnemyConfig"] = [&lua](const std::string& enemyType) -> sol::table {
        sol::function getEnemy = lua["GameAPI"]["GetEnemyConfig"];
        if (getEnemy.valid()) {
            return getEnemy(enemyType);
        }
        return sol::nil;
    };
    
    lua["ConfigAccess"]["GetWeaponConfig"] = [&lua](const std::string& weaponName, 
                                                     sol::optional<int> level) -> sol::table {
        sol::function getWeapon = lua["GameAPI"]["GetWeaponConfig"];
        if (getWeapon.valid()) {
            return getWeapon(weaponName, level.value_or(1));
        }
        return sol::nil;
    };
    
    lua["ConfigAccess"]["GetBossConfig"] = [&lua](const std::string& bossType) -> sol::table {
        sol::function getBoss = lua["GameAPI"]["GetBossConfig"];
        if (getBoss.valid()) {
            return getBoss(bossType);
        }
        return sol::nil;
    };
    
    lua["ConfigAccess"]["GetPowerUpConfig"] = [&lua](const std::string& powerUpType) -> sol::table {
        sol::function getPowerUp = lua["GameAPI"]["GetPowerUpConfig"];
        if (getPowerUp.valid()) {
            return getPowerUp(powerUpType);
        }
        return sol::nil;
    };
}

void GameplayBindings::RegisterGameState(sol::state& lua) {
    // Game state tracking
    lua["GameState"] = lua.create_table();
    lua["GameState"]["currentStage"] = 1;
    lua["GameState"]["currentWave"] = 1;
    lua["GameState"]["stageTime"] = 0.0f;
    lua["GameState"]["score"] = 0;
    lua["GameState"]["lives"] = 3;
    lua["GameState"]["isPlaying"] = false;
    lua["GameState"]["bossActive"] = false;
    
    // Player state
    lua["PlayerState"] = lua.create_table();
    lua["PlayerState"]["health"] = 100;
    lua["PlayerState"]["maxHealth"] = 100;
    lua["PlayerState"]["weapon"] = "single_shot";
    lua["PlayerState"]["weaponLevel"] = 1;
    lua["PlayerState"]["speed"] = 500;
    lua["PlayerState"]["hasForce"] = false;
    lua["PlayerState"]["forceLevel"] = 0;
    lua["PlayerState"]["optionCount"] = 0;
    lua["PlayerState"]["bombs"] = 0;
}

bool GameplayBindings::LoadMasterConfig(sol::state& lua, const std::string& basePath) {
    std::string masterPath = basePath + "config/master_config.lua";
    
    std::cout << "[GameplayBindings] Loading master config from: " << masterPath << std::endl;
    
    try {
        auto result = lua.safe_script_file(masterPath);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[GameplayBindings] Failed to load master config: " << err.what() << std::endl;
            return false;
        }
        
        std::cout << "[GameplayBindings] Master config loaded successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[GameplayBindings] Exception loading master config: " << e.what() << std::endl;
        return false;
    }
}

LuaEnemyConfig GameplayBindings::GetEnemyConfig(sol::state& lua, const std::string& enemyType) {
    LuaEnemyConfig config;
    config.type = enemyType;
    
    sol::function getEnemy = lua["GameAPI"]["GetEnemyConfig"];
    if (!getEnemy.valid()) {
        std::cerr << "[GameplayBindings] GameAPI.GetEnemyConfig not found!" << std::endl;
        return config;
    }
    
    sol::table enemyTable = getEnemy(enemyType);
    if (!enemyTable.valid()) {
        std::cerr << "[GameplayBindings] Enemy type not found: " << enemyType << std::endl;
        return config;
    }
    
    // Extract values from Lua table
    config.name = enemyTable.get_or<std::string>("name", enemyType);
    config.health = enemyTable.get_or<int>("health", 10);
    config.damage = enemyTable.get_or<int>("damage", 10);
    config.speed = enemyTable.get_or<float>("speed", 200.0f);
    config.scoreValue = enemyTable.get_or<int>("scoreValue", 100);
    config.dropChance = enemyTable.get_or<float>("dropChance", 0.0f);
    
    // Movement
    sol::table movement = enemyTable["movement"];
    if (movement.valid()) {
        config.movementPattern = movement.get_or<std::string>("pattern", "straight");
        config.amplitude = movement.get_or<float>("amplitude", 0.0f);
        config.frequency = movement.get_or<float>("frequency", 0.0f);
    }
    
    // Weapon
    config.weaponType = enemyTable.get_or<std::string>("weapon", "");
    config.shootInterval = enemyTable.get_or<float>("shootInterval", 0.0f);
    
    // Sprite
    sol::table sprite = enemyTable["sprite"];
    if (sprite.valid()) {
        config.texture = sprite.get_or<std::string>("texture", "");
        config.frameWidth = sprite.get_or<int>("frameWidth", 32);
        config.frameHeight = sprite.get_or<int>("frameHeight", 32);
        config.scale = sprite.get_or<float>("scale", 2.0f);
    }
    
    // Animation
    sol::table anim = enemyTable["animation"];
    if (anim.valid()) {
        config.frameCount = anim.get_or<int>("frameCount", 1);
        config.frameTime = anim.get_or<float>("frameTime", 0.1f);
    }
    
    // Drop table
    sol::table drops = enemyTable["dropTable"];
    if (drops.valid()) {
        for (auto& pair : drops) {
            if (pair.second.is<std::string>()) {
                config.dropTable.push_back(pair.second.as<std::string>());
            }
        }
    }
    
    return config;
}

LuaBossConfig GameplayBindings::GetBossConfig(sol::state& lua, const std::string& bossType) {
    LuaBossConfig config;
    config.type = bossType;
    
    sol::function getBoss = lua["GameAPI"]["GetBossConfig"];
    if (!getBoss.valid()) {
        std::cerr << "[GameplayBindings] GameAPI.GetBossConfig not found!" << std::endl;
        return config;
    }
    
    sol::table bossTable = getBoss(bossType);
    if (!bossTable.valid()) {
        std::cerr << "[GameplayBindings] Boss type not found: " << bossType << std::endl;
        return config;
    }
    
    // Extract values
    config.name = bossTable.get_or<std::string>("name", bossType);
    config.health = bossTable.get_or<int>("health", 500);
    config.maxHealth = bossTable.get_or<int>("maxHealth", 500);
    config.scoreValue = bossTable.get_or<int>("scoreValue", 10000);
    
    // Entry
    sol::table entry = bossTable["entry"];
    if (entry.valid()) {
        config.entryStartX = entry.get_or<float>("startX", 2200.0f);
        config.entryTargetX = entry.get_or<float>("targetX", 1450.0f);
        config.entryDuration = entry.get_or<float>("duration", 4.0f);
    }
    
    // Movement
    sol::table movement = bossTable["movement"];
    if (movement.valid()) {
        config.movementPattern = movement.get_or<std::string>("pattern", "hover");
        config.amplitude = movement.get_or<float>("amplitude", 80.0f);
        config.frequency = movement.get_or<float>("frequency", 0.5f);
    }
    
    // Sprite
    sol::table sprite = bossTable["sprite"];
    if (sprite.valid()) {
        config.texture = sprite.get_or<std::string>("texture", "");
        config.frameWidth = sprite.get_or<int>("frameWidth", 160);
        config.frameHeight = sprite.get_or<int>("frameHeight", 128);
        config.scale = sprite.get_or<float>("scale", 2.0f);
    }
    
    // Phases
    sol::table phases = bossTable["phases"];
    if (phases.valid()) {
        config.phaseCount = static_cast<int>(phases.size());
        for (auto& pair : phases) {
            sol::table phase = pair.second;
            if (phase.valid()) {
                config.phaseThresholds.push_back(phase.get_or<float>("healthThreshold", 1.0f));
            }
        }
    }
    
    return config;
}

LuaWeaponConfig GameplayBindings::GetWeaponConfig(sol::state& lua, const std::string& weaponName, int level) {
    LuaWeaponConfig config;
    config.name = weaponName;
    
    sol::function getWeapon = lua["GameAPI"]["GetWeaponConfig"];
    if (!getWeapon.valid()) {
        std::cerr << "[GameplayBindings] GameAPI.GetWeaponConfig not found!" << std::endl;
        return config;
    }
    
    sol::table weaponTable = getWeapon(weaponName, level);
    if (!weaponTable.valid()) {
        std::cerr << "[GameplayBindings] Weapon not found: " << weaponName << std::endl;
        return config;
    }
    
    config.fireRate = weaponTable.get_or<float>("fireRate", 0.2f);
    config.projectileSpeed = weaponTable.get_or<float>("projectileSpeed", 1000.0f);
    config.damage = weaponTable.get_or<int>("damage", 10);
    config.projectileCount = weaponTable.get_or<int>("projectileCount", 1);
    config.spreadAngle = weaponTable.get_or<float>("spreadAngle", 0.0f);
    config.canCharge = weaponTable.get_or<bool>("canCharge", false);
    config.maxChargeTime = weaponTable.get_or<float>("maxChargeTime", 1.0f);
    config.piercing = weaponTable.get_or<bool>("piercing", false);
    config.homing = weaponTable.get_or<bool>("homing", false);
    config.homingStrength = weaponTable.get_or<float>("homingStrength", 0.0f);
    
    return config;
}

LuaWaveInfo GameplayBindings::GetActiveWave(sol::state& lua, int stageNumber, float stageTime) {
    LuaWaveInfo info;
    
    sol::function getWaveInfo = lua["GameAPI"]["GetActiveWaveInfo"];
    if (!getWaveInfo.valid()) {
        return info;
    }
    
    sol::table waveTable = getWaveInfo(stageNumber, stageTime);
    if (!waveTable.valid()) {
        return info;
    }
    
    info.name = waveTable.get_or<std::string>("name", "Unknown");
    info.index = waveTable.get_or<int>("index", 0);
    info.isBossWave = waveTable.get_or<bool>("isBossWave", false);
    info.bossType = waveTable.get_or<std::string>("boss", "");
    info.startTime = waveTable.get_or<float>("startTime", 0.0f);
    info.duration = waveTable.get_or<float>("duration", 30.0f);
    
    return info;
}

void GameplayBindings::StartStage(sol::state& lua, int stageNumber) {
    sol::function startStage = lua["SpawnManager"]["StartStage"];
    if (startStage.valid()) {
        startStage(stageNumber);
    }
    
    lua["GameState"]["currentStage"] = stageNumber;
    lua["GameState"]["stageTime"] = 0.0f;
    lua["GameState"]["isPlaying"] = true;
}

std::vector<LuaSpawnRequest> GameplayBindings::UpdateSpawns(sol::state& lua, float deltaTime) {
    std::vector<LuaSpawnRequest> requests;
    
    // Update stage time
    float currentTime = lua["GameState"]["stageTime"];
    lua["GameState"]["stageTime"] = currentTime + deltaTime;
    
    // Call SpawnManager.Update
    sol::function update = lua["SpawnManager"]["Update"];
    if (!update.valid()) {
        return requests;
    }
    
    sol::table spawns = update(deltaTime);
    if (!spawns.valid()) {
        return requests;
    }
    
    // Convert spawns to requests
    for (auto& pair : spawns) {
        sol::table spawn = pair.second;
        if (spawn.valid()) {
            LuaSpawnRequest req;
            req.entityType = "enemy";
            req.subType = spawn.get_or<std::string>("enemy", "basic");
            req.x = spawn.get_or<float>("x", 1920.0f);
            req.y = spawn.get_or<float>("y", 540.0f);
            req.pattern = spawn.get_or<std::string>("pattern", "straight");
            requests.push_back(req);
        }
    }
    
    return requests;
}

std::string GameplayBindings::CheckBossSpawn(sol::state& lua) {
    sol::function checkBoss = lua["SpawnManager"]["ShouldSpawnBoss"];
    if (!checkBoss.valid()) {
        return "";
    }
    
    sol::object result = checkBoss();
    if (result.is<std::string>()) {
        return result.as<std::string>();
    }
    
    return "";
}

void GameplayBindings::SetDifficulty(sol::state& lua, const std::string& difficulty) {
    sol::function setDiff = lua["GameAPI"]["SetDifficulty"];
    if (setDiff.valid()) {
        setDiff(difficulty);
    }
}

std::string GameplayBindings::GetDifficulty(sol::state& lua) {
    sol::table config = lua["GameplayConfig"];
    if (config.valid()) {
        return config.get_or<std::string>("currentDifficulty", "normal");
    }
    return "normal";
}

} // namespace Scripting
} // namespace RType
