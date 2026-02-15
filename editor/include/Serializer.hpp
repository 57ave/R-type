#pragma once

#include "StageData.hpp"

#include <string>
#include <vector>

namespace Editor {

class Serializer {
public:
    static std::string SerializeStages(const std::vector<StageData>& stages,
                                       const std::string& helperBlock);

private:
    static std::string SerializeStage(const StageData& stage, const std::string& indent);
    static std::string SerializeWave(const WaveData& wave, const std::string& indent);
    static std::string SerializeSpawn(const SpawnData& spawn);
    static std::string FormatFloat(float value);
};

}
