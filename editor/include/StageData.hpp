#pragma once

#include <string>
#include <vector>

namespace Editor {

struct WaveEnemy {
    int type = 0;
    int count = 1;
    float interval = 1.0f;
};

struct WaveData {
    float time = 0.0f;
    std::vector<WaveEnemy> enemies;
    bool expanded = true;
};

struct SpawnConfig {
    float enemyInterval = 2.5f;
    float powerupInterval = 15.0f;
    float moduleInterval = 25.0f;
    int maxEnemies = 8;
};

struct BossSpriteData {
    std::string path;
    int frameWidth = 0;
    int frameHeight = 0;
    int frameCount = 1;
    float frameTime = 0.15f;
    float scale = 1.5f;
    bool vertical = false;
};

struct BossData {
    float spawnTime = 90.0f;
    int type = 3;
    std::string name;
    int health = 200;
    float speed = 80.0f;
    float fireRate = 2.0f;
    int firePattern = 0;
    BossSpriteData sprite;
};

struct LevelData {
    int id = 1;
    std::string name;
    std::vector<int> enemyTypes;
    std::vector<int> moduleTypes;
    SpawnConfig spawn;
    std::vector<WaveData> waves;
    BossData boss;
    bool stopSpawningAtBoss = true;
    std::string filePath;
};

struct EnemyTypeInfo {
    std::string key;
    std::string name;
    int health = 10;
    int damage = 20;
    float speed = 200.0f;
    int score = 50;
    std::string spritePath;
    int frameWidth = 32;
    int frameHeight = 32;
    float scaleX = 2.0f;
    float scaleY = 2.0f;
    std::string movementType;
};

struct EditorData {
    std::vector<LevelData> levels;
    std::vector<EnemyTypeInfo> enemyTypes;
    std::string levelsDir;
    std::string enemiesConfigPath;
    std::string assetsBasePath;

    int selectedLevelIndex = 0;
    int selectedWaveIndex = 0;
    bool dirty = false;
};

inline const char* EnemyTypeNames[] = {"bug", "fighter", "kamikaze", "drone"};
inline constexpr int EnemyTypeNameCount = 4;

inline const char* ModuleTypeLabels[] = {"(0)", "homing", "(2)", "spread", "wave"};
inline constexpr int ModuleTypeLabelCount = 5;

inline const int FirePatternValues[] = {0, 2, 3};
inline const char* FirePatternNames[] = {"straight (0)", "circle (2)", "spread (3)"};
inline constexpr int FirePatternCount = 3;

}
