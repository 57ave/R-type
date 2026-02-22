#ifndef SHOOTEMUP_COMPONENTS_WAVE_HPP
#define SHOOTEMUP_COMPONENTS_WAVE_HPP

#include <string>
#include <vector>

namespace ShootEmUp {
namespace Components {

/**
 * @brief Definition of an enemy spawn within a wave
 */
struct EnemySpawnInfo {
    std::string enemyType = "basic";
    float spawnTime = 0.0f;      // Time after wave start
    float spawnX = 1920.0f;      // Spawn position X
    float spawnY = 540.0f;       // Spawn position Y
    std::string pattern = "straight";
    int count = 1;               // Number of this enemy to spawn
    float spacing = 0.5f;        // Time between each spawn if count > 1
    std::string formation = "single";  // "single", "line", "v_formation", "circle"
};

/**
 * @brief Wave component - defines a single wave of enemies
 */
struct Wave {
    int waveNumber = 1;
    std::string waveName = "Wave 1";
    
    // Wave timing
    float duration = 30.0f;      // Max wave duration
    float currentTime = 0.0f;
    bool isActive = false;
    bool isCompleted = false;
    
    // Enemy spawns
    std::vector<EnemySpawnInfo> spawns;
    int currentSpawnIndex = 0;
    int enemiesSpawned = 0;
    int enemiesKilled = 0;
    int totalEnemies = 0;
    
    // Wave completion conditions
    bool requireAllKilled = true;     // Must kill all enemies
    bool hasTimeLimit = false;        // Fail if time runs out
    float timeLimitBonus = 1000.0f;   // Bonus for fast completion
    
    // Boss wave
    bool isBossWave = false;
    std::string bossType = "";
    
    // Difficulty modifiers
    float enemyHealthMultiplier = 1.0f;
    float enemySpeedMultiplier = 1.0f;
    float enemyFireRateMultiplier = 1.0f;
    
    // Rewards
    int completionScore = 500;
    std::string powerUpReward = "";  // Power-up type dropped at end
};

/**
 * @brief Stage/Level component - contains multiple waves
 */
struct Stage {
    int stageNumber = 1;
    std::string stageName = "Stage 1";
    std::string backgroundMusic = "";
    std::string backgroundTexture = "";
    
    // Waves in this stage
    std::vector<Wave> waves;
    int currentWaveIndex = 0;
    
    // Stage timing
    float timeBetweenWaves = 3.0f;
    float waveTransitionTimer = 0.0f;
    bool inTransition = false;
    
    // Stage state
    bool isActive = false;
    bool isCompleted = false;
    bool isFailed = false;
    
    // Stage completion
    int totalScore = 0;
    int livesLost = 0;
    float completionTime = 0.0f;
    
    // Difficulty
    int difficultyLevel = 1;  // 1=Easy, 2=Normal, 3=Hard
    
    // Bonus conditions
    bool noDeathBonus = true;
    int noDeathBonusValue = 5000;
    bool speedBonus = true;
    float speedBonusThreshold = 120.0f;  // Complete under 2 minutes
    int speedBonusValue = 3000;
};

/**
 * @brief Game progress tracking
 */
struct GameProgress {
    int currentStage = 1;
    int maxStages = 1;
    int totalScore = 0;
    int lives = 3;
    int continues = 2;
    
    // Power-up state (persists between stages)
    std::string currentWeapon = "";
    int weaponLevel = 1;
    bool hasShield = false;
    int speedLevel = 0;  // speed upgrades
    
    // Statistics
    int enemiesKilled = 0;
    int bossesDefeated = 0;
    int powerUpsCollected = 0;
    float totalPlayTime = 0.0f;
    
    // Achievements
    bool perfectStage = false;  // No deaths in a stage
    bool speedRunner = false;   // Beat stage under time
    bool bossSlayer = false;    // Defeated boss without taking damage
};

} // namespace Components
} // namespace ShootEmUp

#endif // SHOOTEMUP_COMPONENTS_WAVE_HPP
