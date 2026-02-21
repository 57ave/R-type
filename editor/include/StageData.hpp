#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Editor {

struct SpawnData {
    float time = 0.0f;
    std::string enemy = "basic";
    float y = 400.0f;
    std::string pattern = "straight";
    int count = 1;
    float spacing = 0.3f;

    bool selected = false;
    int editorId = 0;
};

struct RewardData {
    std::string type;
    float y = 400.0f;
};

struct WaveData {
    std::string name = "New Wave";
    float startTime = 0.0f;
    float duration = 30.0f;
    std::vector<SpawnData> spawns;
    std::optional<RewardData> reward;

    bool isBossWave = false;
    std::string boss;

    bool expanded = true;
};

struct BackgroundData {
    std::string texture = "background.png";
    float scrollSpeed = 200.0f;
};

struct StageData {
    std::string key;
    std::string name = "New Stage";
    std::string description;
    int stageNumber = 1;
    BackgroundData background;
    std::string music;
    std::string bossMusic;
    float duration = 180.0f;
    std::vector<WaveData> waves;

    int completionBonus = 5000;
    int perfectBonus = 10000;
    float speedBonusTime = 120.0f;
    int speedBonus = 3000;
};

struct EnemyTypeInfo {
    std::string key;
    std::string name;
    std::string category;
    int health = 1;
    float speed = 200.0f;

    std::string texture;
    int frameWidth = 32;
    int frameHeight = 32;
    float scale = 2.0f;
};

struct EditorData {
    std::vector<StageData> stages;
    std::vector<EnemyTypeInfo> enemyTypes;
    std::string configFilePath;
    std::string enemiesConfigFilePath;
    std::string assetsBasePath;

    int selectedStageIndex = 0;
    int selectedWaveIndex = 0;
    int selectedSpawnIndex = -1;
    bool dirty = false;

    int nextSpawnId = 1;
};

inline const char* KnownPatterns[] = {
    "straight", "zigzag", "sinewave", "chase",
    "evasive", "stationary", "hover"
};
inline constexpr int KnownPatternCount = 7;

}
