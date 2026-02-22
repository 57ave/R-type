#include "Serializer.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Editor {

std::string Serializer::FormatFloat(float value) {
    if (std::abs(value - std::round(value)) < 0.001f) {
        std::ostringstream ss;
        ss << static_cast<int>(std::round(value)) << ".0";
        return ss.str();
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", value);
    std::string s = buf;
    size_t dot = s.find('.');
    if (dot != std::string::npos) {
        size_t last = s.find_last_not_of('0');
        if (last != std::string::npos && last > dot) {
            s = s.substr(0, last + 1);
        }
    }
    return s;
}

std::string Serializer::SerializeLevel(const LevelData& level) {
    std::ostringstream ss;
    std::string varName = "Level" + std::to_string(level.id);
    std::string i1 = "    ";
    std::string i2 = "        ";
    std::string i3 = "            ";

    ss << "-- ==========================================\n";
    ss << "-- R-Type Game - Level " << level.id << ": " << level.name << "\n";
    ss << "-- ==========================================\n";
    ss << "\n";
    ss << varName << " = {\n";
    ss << i1 << "id = " << level.id << ",\n";
    ss << i1 << "name = \"" << level.name << "\",\n";
    ss << "\n";

    ss << i1 << "enemy_types = {";
    for (size_t i = 0; i < level.enemyTypes.size(); i++) {
        if (i > 0) ss << ", ";
        ss << level.enemyTypes[i];
    }
    ss << "},\n";

    ss << i1 << "module_types = {";
    for (size_t i = 0; i < level.moduleTypes.size(); i++) {
        if (i > 0) ss << ", ";
        ss << level.moduleTypes[i];
    }
    ss << "},\n";
    ss << "\n";

    ss << i1 << "enemy_interval = " << FormatFloat(level.spawn.enemyInterval) << ",\n";
    ss << i1 << "powerup_interval = " << FormatFloat(level.spawn.powerupInterval) << ",\n";
    ss << i1 << "module_interval = " << FormatFloat(level.spawn.moduleInterval) << ",\n";
    ss << i1 << "max_enemies = " << level.spawn.maxEnemies << ",\n";
    ss << i1 << "stop_spawning_at_boss = " << (level.stopSpawningAtBoss ? "true" : "false") << ",\n";
    ss << "\n";

    ss << i1 << "waves = {\n";
    for (size_t w = 0; w < level.waves.size(); w++) {
        const auto& wave = level.waves[w];
        ss << i2 << "{\n";
        ss << i3 << "time = " << FormatFloat(wave.time) << ",\n";
        ss << i3 << "groups = {\n";
        for (size_t e = 0; e < wave.enemies.size(); e++) {
            const auto& enemy = wave.enemies[e];
            ss << i3 << "    {type = " << enemy.type
               << ", count = " << enemy.count
               << ", interval = " << FormatFloat(enemy.interval) << "},\n";
        }
        ss << i3 << "}\n";
        ss << i2 << "},\n";
    }
    ss << i1 << "},\n";
    ss << "\n";

    ss << i1 << "boss = {\n";
    ss << i2 << "spawn_time = " << FormatFloat(level.boss.spawnTime) << ",\n";
    ss << i2 << "enemy_type = " << level.boss.type << ",\n";
    ss << i2 << "name = \"" << level.boss.name << "\",\n";
    ss << i2 << "health = " << level.boss.health << ",\n";
    ss << i2 << "speed = " << FormatFloat(level.boss.speed) << ",\n";
    ss << i2 << "fire_rate = " << FormatFloat(level.boss.fireRate) << ",\n";
    ss << i2 << "fire_pattern = " << level.boss.firePattern << ",\n";
    ss << i2 << "sprite = {\n";
    ss << i3 << "path = \"" << level.boss.sprite.path << "\",\n";
    ss << i3 << "frame_width = " << level.boss.sprite.frameWidth << ",\n";
    ss << i3 << "frame_height = " << level.boss.sprite.frameHeight << ",\n";
    ss << i3 << "frame_count = " << level.boss.sprite.frameCount << ",\n";
    ss << i3 << "frame_time = " << FormatFloat(level.boss.sprite.frameTime) << ",\n";
    ss << i3 << "scale = " << FormatFloat(level.boss.sprite.scale) << ",\n";
    ss << i3 << "vertical = " << (level.boss.sprite.vertical ? "true" : "false") << ",\n";
    ss << i2 << "},\n";
    ss << i1 << "},\n";
    ss << "\n";
    ss << "}\n";
    ss << "\n";
    ss << "print(\"[LUA] Level " << level.id << " loaded\")\n";
    ss << "return " << varName << "\n";

    return ss.str();
}

bool Serializer::SaveLevel(const LevelData& level, const std::string& path) {
    std::string content = SerializeLevel(level);

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Serializer] Cannot write to: " << path << std::endl;
        return false;
    }

    file << content;
    file.close();

    std::cout << "[Serializer] Saved level " << level.id << " to " << path << std::endl;
    return true;
}

}
