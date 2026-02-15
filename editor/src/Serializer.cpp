#include "Serializer.hpp"

#include <cmath>
#include <sstream>

namespace Editor {

std::string Serializer::FormatFloat(float value) {
    if (std::abs(value - std::round(value)) < 0.001f) {
        std::ostringstream ss;
        ss << static_cast<int>(std::round(value)) << ".0";
        return ss.str();
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f", value);
    return buf;
}

std::string Serializer::SerializeSpawn(const SpawnData& spawn) {
    std::string line = "{ time = " + FormatFloat(spawn.time) + ",  enemy = \"" + spawn.enemy +
                       "\", y = " + FormatFloat(spawn.y) + ", pattern = \"" + spawn.pattern + "\"";
    if (spawn.count > 1) {
        line += ", count = " + std::to_string(spawn.count) + ", spacing = " +
                FormatFloat(spawn.spacing);
    }
    line += " }";
    return line;
}

std::string Serializer::SerializeWave(const WaveData& wave, const std::string& indent) {
    std::ostringstream ss;
    std::string i2 = indent + "    ";
    std::string i3 = i2 + "    ";

    ss << indent << "{\n";
    ss << i2 << "name = \"" << wave.name << "\",\n";
    ss << i2 << "startTime = " << FormatFloat(wave.startTime) << ",\n";
    ss << i2 << "duration = " << FormatFloat(wave.duration) << ",\n";

    if (wave.isBossWave) {
        ss << i2 << "isBossWave = true,\n";
        if (!wave.boss.empty()) {
            ss << i2 << "boss = \"" << wave.boss << "\",\n";
        }
    }

    ss << i2 << "\n";
    ss << i2 << "spawns = {\n";
    for (size_t j = 0; j < wave.spawns.size(); j++) {
        ss << i3 << SerializeSpawn(wave.spawns[j]);
        if (j + 1 < wave.spawns.size()) {
            ss << ",";
        }
        ss << "\n";
    }
    ss << i2 << "}";

    if (wave.reward) {
        ss << ",\n";
        ss << i2 << "\n";
        ss << i2 << "reward = { type = \"" << wave.reward->type << "\", y = "
           << FormatFloat(wave.reward->y) << " }";
    }

    ss << "\n" << indent << "}";
    return ss.str();
}

std::string Serializer::SerializeStage(const StageData& stage, const std::string& indent) {
    std::ostringstream ss;
    std::string i2 = indent + "    ";
    std::string i3 = i2 + "    ";

    ss << indent << stage.key << " = {\n";
    ss << i2 << "name = \"" << stage.name << "\",\n";
    ss << i2 << "description = \"" << stage.description << "\",\n";
    ss << i2 << "stageNumber = " << stage.stageNumber << ",\n";
    ss << i2 << "\n";

    ss << i2 << "background = {\n";
    ss << i3 << "texture = \"" << stage.background.texture << "\",\n";
    ss << i3 << "scrollSpeed = " << FormatFloat(stage.background.scrollSpeed) << "\n";
    ss << i2 << "},\n";
    ss << i2 << "\n";

    ss << i2 << "music = \"" << stage.music << "\",\n";
    if (!stage.bossMusic.empty()) {
        ss << i2 << "bossMusic = \"" << stage.bossMusic << "\",\n";
    }
    ss << i2 << "\n";

    ss << i2 << "duration = " << FormatFloat(stage.duration) << ",\n";
    ss << i2 << "\n";

    ss << i2 << "waves = {\n";
    for (size_t w = 0; w < stage.waves.size(); w++) {
        ss << SerializeWave(stage.waves[w], i3);
        if (w + 1 < stage.waves.size()) {
            ss << ",";
        }
        ss << "\n";
    }
    ss << i2 << "},\n";
    ss << i2 << "\n";

    ss << i2 << "completionBonus = " << stage.completionBonus << ",\n";
    ss << i2 << "perfectBonus = " << stage.perfectBonus << ",\n";
    ss << i2 << "speedBonusTime = " << FormatFloat(stage.speedBonusTime) << ",\n";
    ss << i2 << "speedBonus = " << stage.speedBonus << "\n";

    ss << indent << "}";
    return ss.str();
}

std::string Serializer::SerializeStages(const std::vector<StageData>& stages,
                                        const std::string& helperBlock) {
    std::ostringstream ss;

    ss << "-- ============================================================================\n";
    ss << "-- STAGES AND WAVES CONFIGURATION\n";
    ss << "-- Complete level/wave definitions - data-driven level design\n";
    ss << "-- ============================================================================\n";
    ss << "\n";
    ss << "StagesConfig = {\n";

    for (size_t i = 0; i < stages.size(); i++) {
        if (i > 0) {
            ss << ",\n    \n";
        }

        ss << "    -- ========================================================================\n";
        ss << "    -- STAGE " << stages[i].stageNumber << " - ";
        std::string upper = stages[i].name;
        for (auto& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        ss << upper << "\n";
        ss << "    -- ========================================================================\n";

        ss << SerializeStage(stages[i], "    ");
    }

    ss << "\n}\n";

    if (!helperBlock.empty()) {
        ss << helperBlock;
    }

    return ss.str();
}

}
