#include "LuaParser.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace Editor {

std::string LuaParser::lastError_;

const std::string& LuaParser::GetLastError() {
    return lastError_;
}

static std::string SolStr(const sol::table& t, const char* key,
                          const std::string& def = "") {
    sol::optional<std::string> v = t[key];
    return v.value_or(def);
}

static float SolFloat(const sol::table& t, const char* key, float def = 0.0f) {
    sol::optional<double> v = t[key];
    return v ? static_cast<float>(v.value()) : def;
}

static int SolInt(const sol::table& t, const char* key, int def = 0) {
    sol::optional<int> v = t[key];
    return v.value_or(def);
}

static bool SolBool(const sol::table& t, const char* key, bool def = false) {
    sol::optional<bool> v = t[key];
    return v.value_or(def);
}

static std::vector<int> SolIntArray(const sol::table& t, const char* key) {
    std::vector<int> result;
    sol::optional<sol::table> arr = t[key];
    if (arr) {
        for (size_t i = 1; i <= arr->size(); i++) {
            sol::optional<int> v = (*arr)[i];
            if (v) result.push_back(*v);
        }
    }
    return result;
}

bool LuaParser::LoadSingleLevel(const std::string& path, LevelData& outLevel) {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string,
                       sol::lib::table, sol::lib::io);

    try {
        auto result = lua.safe_script_file(path);
        if (!result.valid()) {
            sol::error err = result;
            lastError_ = std::string("Lua parse error: ") + err.what();
            return false;
        }
    } catch (const sol::error& e) {
        lastError_ = std::string("Lua exception: ") + e.what();
        return false;
    }

    // Find the LevelN table by trying Level1, Level2, ... Level10
    sol::table levelTable;
    int foundId = 0;
    for (int i = 1; i <= 10; i++) {
        std::string name = "Level" + std::to_string(i);
        sol::object obj = lua[name];
        if (obj.valid() && obj.get_type() == sol::type::table) {
            levelTable = obj.as<sol::table>();
            foundId = i;
            break;
        }
    }

    if (!levelTable.valid()) {
        lastError_ = "No LevelN table found in " + path;
        return false;
    }

    outLevel.filePath = path;
    outLevel.id = SolInt(levelTable, "id", foundId);
    outLevel.name = SolStr(levelTable, "name", "Level " + std::to_string(outLevel.id));
    outLevel.enemyTypes = SolIntArray(levelTable, "enemy_types");
    outLevel.moduleTypes = SolIntArray(levelTable, "module_types");
    outLevel.stopSpawningAtBoss = SolBool(levelTable, "stop_spawning_at_boss", true);

    // Parse spawn config
    sol::optional<sol::table> spawnTable = levelTable["spawn"];
    if (spawnTable) {
        outLevel.spawn.enemyInterval = SolFloat(*spawnTable, "enemy_interval", 2.5f);
        outLevel.spawn.powerupInterval = SolFloat(*spawnTable, "powerup_interval", 15.0f);
        outLevel.spawn.moduleInterval = SolFloat(*spawnTable, "module_interval", 25.0f);
        outLevel.spawn.maxEnemies = SolInt(*spawnTable, "max_enemies", 8);
    }

    // Parse waves
    sol::optional<sol::table> wavesTable = levelTable["waves"];
    if (wavesTable) {
        for (size_t i = 1; i <= wavesTable->size(); i++) {
            sol::optional<sol::table> wt = (*wavesTable)[i];
            if (!wt) continue;

            WaveData wave;
            wave.time = SolFloat(*wt, "time");

            sol::optional<sol::table> enemiesArr = (*wt)["enemies"];
            if (enemiesArr) {
                for (size_t j = 1; j <= enemiesArr->size(); j++) {
                    sol::optional<sol::table> et = (*enemiesArr)[j];
                    if (!et) continue;

                    WaveEnemy enemy;
                    enemy.type = SolInt(*et, "type", 0);
                    enemy.count = SolInt(*et, "count", 1);
                    enemy.interval = SolFloat(*et, "interval", 1.0f);
                    wave.enemies.push_back(enemy);
                }
            }

            outLevel.waves.push_back(wave);
        }
    }

    // Parse boss
    sol::optional<sol::table> bossTable = levelTable["boss"];
    if (bossTable) {
        outLevel.boss.spawnTime = SolFloat(*bossTable, "spawn_time", 90.0f);
        outLevel.boss.type = SolInt(*bossTable, "type", 3);
        outLevel.boss.name = SolStr(*bossTable, "name");
        outLevel.boss.health = SolInt(*bossTable, "health", 200);
        outLevel.boss.speed = SolFloat(*bossTable, "speed", 80.0f);
        outLevel.boss.fireRate = SolFloat(*bossTable, "fire_rate", 2.0f);
        outLevel.boss.firePattern = SolInt(*bossTable, "fire_pattern", 0);

        sol::optional<sol::table> spriteTable = (*bossTable)["sprite"];
        if (spriteTable) {
            outLevel.boss.sprite.path = SolStr(*spriteTable, "path");
            outLevel.boss.sprite.frameWidth = SolInt(*spriteTable, "frame_width", 0);
            outLevel.boss.sprite.frameHeight = SolInt(*spriteTable, "frame_height", 0);
            outLevel.boss.sprite.frameCount = SolInt(*spriteTable, "frame_count", 1);
            outLevel.boss.sprite.frameTime = SolFloat(*spriteTable, "frame_time", 0.15f);
            outLevel.boss.sprite.scale = SolFloat(*spriteTable, "scale", 1.5f);
            outLevel.boss.sprite.vertical = SolBool(*spriteTable, "vertical", false);
        }
    }

    std::cout << "[LuaParser] Loaded level " << outLevel.id << " (" << outLevel.name
              << ") from " << path << std::endl;
    return true;
}

bool LuaParser::LoadLevels(const std::string& dir, std::vector<LevelData>& outLevels) {
    outLevels.clear();
    namespace fs = std::filesystem;

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        lastError_ = "Levels directory not found: " + dir;
        return false;
    }

    std::vector<std::string> levelFiles;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("level_") == 0 && filename.find(".lua") != std::string::npos) {
                levelFiles.push_back(entry.path().string());
            }
        }
    }

    std::sort(levelFiles.begin(), levelFiles.end());

    for (const auto& file : levelFiles) {
        LevelData level;
        if (LoadSingleLevel(file, level)) {
            outLevels.push_back(level);
        } else {
            std::cerr << "[LuaParser] Warning: failed to load " << file
                      << ": " << lastError_ << std::endl;
        }
    }

    std::sort(outLevels.begin(), outLevels.end(),
              [](const LevelData& a, const LevelData& b) {
                  return a.id < b.id;
              });

    std::cout << "[LuaParser] Loaded " << outLevels.size() << " levels from " << dir << std::endl;
    return !outLevels.empty();
}

bool LuaParser::LoadEnemies(const std::string& path, std::vector<EnemyTypeInfo>& outEnemies) {
    outEnemies.clear();

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

    try {
        auto result = lua.safe_script_file(path);
        if (!result.valid()) {
            sol::error err = result;
            lastError_ = std::string("Lua parse error: ") + err.what();
            return false;
        }
    } catch (const sol::error& e) {
        lastError_ = std::string("Lua exception: ") + e.what();
        return false;
    }

    sol::table enemiesTable = lua["EnemiesSimple"];
    if (!enemiesTable.valid()) {
        lastError_ = "EnemiesSimple table not found in " + path;
        return false;
    }

    for (auto& [key, value] : enemiesTable) {
        if (value.get_type() != sol::type::table) continue;

        std::string keyStr = key.as<std::string>();
        // Skip non-enemy entries like enemy_projectiles
        if (keyStr == "enemy_projectiles") continue;

        EnemyTypeInfo info;
        info.key = keyStr;
        sol::table t = value.as<sol::table>();

        info.name = SolStr(t, "name", info.key);
        info.health = SolInt(t, "health", 10);
        info.damage = SolInt(t, "damage", 20);
        info.speed = SolFloat(t, "speed", 200.0f);
        info.score = SolInt(t, "score", 50);

        sol::optional<sol::table> sprite = t["sprite"];
        if (sprite) {
            info.spritePath = SolStr(*sprite, "path");
            sol::optional<sol::table> scale = (*sprite)["scale"];
            if (scale) {
                sol::optional<double> sx = (*scale)[1];
                sol::optional<double> sy = (*scale)[2];
                if (sx) info.scaleX = static_cast<float>(*sx);
                if (sy) info.scaleY = static_cast<float>(*sy);
            }
        }

        sol::optional<sol::table> anim = t["animation"];
        if (anim) {
            info.frameWidth = SolInt(*anim, "frame_width", 32);
            info.frameHeight = SolInt(*anim, "frame_height", 32);
        }

        sol::optional<sol::table> movement = t["movement"];
        if (movement) {
            info.movementType = SolStr(*movement, "type", "straight");
        }

        outEnemies.push_back(info);
    }

    std::sort(outEnemies.begin(), outEnemies.end(),
              [](const EnemyTypeInfo& a, const EnemyTypeInfo& b) {
                  return a.key < b.key;
              });

    std::cout << "[LuaParser] Loaded " << outEnemies.size() << " enemy types from " << path
              << std::endl;
    return true;
}

}
