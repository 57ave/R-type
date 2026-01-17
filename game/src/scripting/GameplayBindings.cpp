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
    config.name = enemyTable["name"].get_or(enemyType);
    config.health = enemyTable["health"].get_or(10);
    config.damage = enemyTable["damage"].get_or(10);
    config.speed = enemyTable["speed"].get_or(200.0f);
    config.scoreValue = enemyTable["scoreValue"].get_or(100);
    config.dropChance = enemyTable["dropChance"].get_or(0.0f);
    
    // Movement
    sol::table movement = enemyTable["movement"];
    if (movement.valid()) {
    config.movementPattern = movement["pattern"].get_or(std::string("straight"));
    config.amplitude = movement["amplitude"].get_or(0.0f);
    config.frequency = movement["frequency"].get_or(0.0f);
    }
    
    // Weapon
    config.weaponType = enemyTable["weapon"].get_or(std::string(""));
    config.shootInterval = enemyTable["shootInterval"].get_or(0.0f);
    
    // Sprite
    sol::table sprite = enemyTable["sprite"];
    if (sprite.valid()) {
    config.texture = sprite["texture"].get_or(std::string(""));
    config.frameWidth = sprite["frameWidth"].get_or(32);
    config.frameHeight = sprite["frameHeight"].get_or(32);
    config.scale = sprite["scale"].get_or(2.0f);
    }
    
    // Animation
    sol::table anim = enemyTable["animation"];
    if (anim.valid()) {
    config.frameCount = anim["frameCount"].get_or(1);
    config.frameTime = anim["frameTime"].get_or(0.1f);
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
    config.name = bossTable["name"].get_or(bossType);
    config.health = bossTable["health"].get_or(500);
    config.maxHealth = bossTable["maxHealth"].get_or(500);
    config.scoreValue = bossTable["scoreValue"].get_or(10000);
    
    // Entry
    sol::table entry = bossTable["entry"];
    if (entry.valid()) {
    config.entryStartX = entry["startX"].get_or(2200.0f);
    config.entryTargetX = entry["targetX"].get_or(1450.0f);
    config.entryDuration = entry["duration"].get_or(4.0f);
    }
    
    // Movement
    sol::table movement = bossTable["movement"];
    if (movement.valid()) {
    config.movementPattern = movement["pattern"].get_or(std::string("hover"));
    config.amplitude = movement["amplitude"].get_or(80.0f);
    config.frequency = movement["frequency"].get_or(0.5f);
    }
    
    // Sprite
    sol::table sprite = bossTable["sprite"];
    if (sprite.valid()) {
    config.texture = sprite["texture"].get_or(std::string(""));
    config.frameWidth = sprite["frameWidth"].get_or(160);
    config.frameHeight = sprite["frameHeight"].get_or(128);
    config.scale = sprite["scale"].get_or(2.0f);
    }
    
    // Phases
    sol::table phases = bossTable["phases"];
    if (phases.valid()) {
        config.phaseCount = static_cast<int>(phases.size());
        for (auto& pair : phases) {
            sol::table phase = pair.second;
            if (phase.valid()) {
                config.phaseThresholds.push_back(phase["healthThreshold"].get_or(1.0f));
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
    
    config.fireRate = weaponTable["fireRate"].get_or(0.2f);
    config.projectileSpeed = weaponTable["projectileSpeed"].get_or(1000.0f);
    config.damage = weaponTable["damage"].get_or(10);
    config.projectileCount = weaponTable["projectileCount"].get_or(1);
    config.spreadAngle = weaponTable["spreadAngle"].get_or(0.0f);
    config.canCharge = weaponTable["canCharge"].get_or(false);
    config.maxChargeTime = weaponTable["maxChargeTime"].get_or(1.0f);
    config.piercing = weaponTable["piercing"].get_or(false);
    config.homing = weaponTable["homing"].get_or(false);
    config.homingStrength = weaponTable["homingStrength"].get_or(0.0f);
    
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
    
    info.name = waveTable["name"].get_or(std::string("Unknown"));
    info.index = waveTable["index"].get_or(0);
    info.isBossWave = waveTable["isBossWave"].get_or(false);
    info.bossType = waveTable["boss"].get_or(std::string(""));
    info.startTime = waveTable["startTime"].get_or(0.0f);
    info.duration = waveTable["duration"].get_or(30.0f);
    
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
            req.subType = spawn["enemy"].get_or(std::string("basic"));
            req.x = spawn["x"].get_or(1920.0f);
            req.y = spawn["y"].get_or(540.0f);
            req.pattern = spawn["pattern"].get_or(std::string("straight"));
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
        return config["currentDifficulty"].get_or(std::string("normal"));
    }
    return "normal";
}

} // namespace Scripting
} // namespace RType
