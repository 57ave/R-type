#include "LuaParser.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

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

bool LuaParser::LoadStages(const std::string& path, std::vector<StageData>& outStages,
                           std::string& outHelperBlock) {
    outStages.clear();

    {
        std::ifstream file(path);
        if (!file.is_open()) {
            lastError_ = "Cannot open file: " + path;
            return false;
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        int braceDepth = 0;
        bool inStagesConfig = false;
        size_t endPos = std::string::npos;

        for (size_t i = 0; i < content.size(); i++) {
            if (content[i] == '"') {
                i++;
                while (i < content.size() && content[i] != '"') {
                    if (content[i] == '\\') i++;
                    i++;
                }
                continue;
            }
            if (content[i] == '-' && i + 1 < content.size() && content[i + 1] == '-') {
                while (i < content.size() && content[i] != '\n') i++;
                continue;
            }

            if (!inStagesConfig) {
                if (i + 12 <= content.size() && content.substr(i, 12) == "StagesConfig") {
                    size_t j = i + 12;
                    while (j < content.size() && (content[j] == ' ' || content[j] == '\t')) j++;
                    if (j < content.size() && content[j] == '=') {
                        inStagesConfig = true;
                        i = j;
                    }
                }
            } else {
                if (content[i] == '{') {
                    braceDepth++;
                } else if (content[i] == '}') {
                    braceDepth--;
                    if (braceDepth == 0) {
                        endPos = i + 1;
                        break;
                    }
                }
            }
        }

        if (endPos != std::string::npos && endPos < content.size()) {
            outHelperBlock = content.substr(endPos);
        } else {
            outHelperBlock = "";
        }
    }

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

    sol::table stagesTable = lua["StagesConfig"];
    if (!stagesTable.valid()) {
        lastError_ = "StagesConfig table not found in " + path;
        return false;
    }

    for (auto& [key, value] : stagesTable) {
        if (value.get_type() != sol::type::table) continue;

        StageData stage;
        stage.key = key.as<std::string>();
        sol::table t = value.as<sol::table>();

        stage.name = SolStr(t, "name");
        stage.description = SolStr(t, "description");
        stage.stageNumber = SolInt(t, "stageNumber", 1);
        stage.duration = SolFloat(t, "duration", 180.0f);
        stage.music = SolStr(t, "music");
        stage.bossMusic = SolStr(t, "bossMusic");

        sol::optional<sol::table> bg = t["background"];
        if (bg) {
            stage.background.texture = SolStr(*bg, "texture");
            stage.background.scrollSpeed = SolFloat(*bg, "scrollSpeed", 200.0f);
        }

        stage.completionBonus = SolInt(t, "completionBonus", 5000);
        stage.perfectBonus = SolInt(t, "perfectBonus", 10000);
        stage.speedBonusTime = SolFloat(t, "speedBonusTime", 120.0f);
        stage.speedBonus = SolInt(t, "speedBonus", 3000);

        sol::optional<sol::table> waves = t["waves"];
        if (waves) {
            for (size_t i = 1; i <= waves->size(); i++) {
                sol::table wt = (*waves)[i];
                WaveData wave;
                wave.name = SolStr(wt, "name");
                wave.startTime = SolFloat(wt, "startTime");
                wave.duration = SolFloat(wt, "duration", 30.0f);
                wave.isBossWave = SolBool(wt, "isBossWave");
                wave.boss = SolStr(wt, "boss");

                sol::optional<sol::table> spawns = wt["spawns"];
                if (spawns) {
                    for (size_t j = 1; j <= spawns->size(); j++) {
                        sol::table st = (*spawns)[j];
                        SpawnData spawn;
                        spawn.time = SolFloat(st, "time");
                        spawn.enemy = SolStr(st, "enemy", "basic");
                        spawn.y = SolFloat(st, "y", 400.0f);
                        spawn.pattern = SolStr(st, "pattern", "straight");
                        spawn.count = SolInt(st, "count", 1);
                        spawn.spacing = SolFloat(st, "spacing", 0.3f);
                        wave.spawns.push_back(spawn);
                    }
                }

                sol::optional<sol::table> reward = wt["reward"];
                if (reward) {
                    RewardData r;
                    r.type = SolStr(*reward, "type");
                    r.y = SolFloat(*reward, "y", 400.0f);
                    wave.reward = r;
                }

                stage.waves.push_back(wave);
            }
        }

        outStages.push_back(stage);
    }

    std::sort(outStages.begin(), outStages.end(),
              [](const StageData& a, const StageData& b) {
                  return a.stageNumber < b.stageNumber;
              });

    std::cout << "[LuaParser] Loaded " << outStages.size() << " stages from " << path << std::endl;
    return true;
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

    sol::table enemiesTable = lua["EnemiesConfig"];
    if (!enemiesTable.valid()) {
        lastError_ = "EnemiesConfig table not found in " + path;
        return false;
    }

    for (auto& [key, value] : enemiesTable) {
        if (value.get_type() != sol::type::table) continue;

        EnemyTypeInfo info;
        info.key = key.as<std::string>();
        sol::table t = value.as<sol::table>();

        info.name = SolStr(t, "name", info.key);
        info.category = SolStr(t, "category", "common");
        info.health = SolInt(t, "health", 1);
        info.speed = SolFloat(t, "speed", 200.0f);

        sol::optional<sol::table> sprite = t["sprite"];
        if (sprite) {
            info.texture = SolStr(*sprite, "texture");
            info.frameWidth = SolInt(*sprite, "frameWidth", 32);
            info.frameHeight = SolInt(*sprite, "frameHeight", 32);
            info.scale = SolFloat(*sprite, "scale", 2.0f);
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
