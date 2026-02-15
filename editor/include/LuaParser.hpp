#pragma once

#include "StageData.hpp"

#include <string>
#include <vector>

namespace Editor {

class LuaParser {
public:
    static bool LoadStages(const std::string& path, std::vector<StageData>& outStages,
                           std::string& outHelperBlock);
    static bool LoadEnemies(const std::string& path, std::vector<EnemyTypeInfo>& outEnemies);
    static const std::string& GetLastError();

private:
    static std::string lastError_;
};

}
