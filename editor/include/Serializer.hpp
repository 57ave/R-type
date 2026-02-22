#pragma once

#include "StageData.hpp"

#include <string>

namespace Editor {

class Serializer {
public:
    static std::string SerializeLevel(const LevelData& level);
    static bool SaveLevel(const LevelData& level, const std::string& path);

private:
    static std::string FormatFloat(float value);
};

}
