#pragma once

#include "StageData.hpp"

#include <string>
#include <vector>

namespace Editor {

class LuaParser {
public:
    static bool LoadLevels(const std::string& dir, std::vector<LevelData>& outLevels);
    static bool LoadEnemies(const std::string& path, std::vector<EnemyTypeInfo>& outEnemies);
    static const std::string& GetLastError();

private:
    static bool LoadSingleLevel(const std::string& path, LevelData& outLevel);
    static std::string lastError_;
};

}
