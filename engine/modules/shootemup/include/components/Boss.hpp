#ifndef RTYPE_ENGINE_COMPONENTS_BOSS_HPP
#define RTYPE_ENGINE_COMPONENTS_BOSS_HPP

#include <string>
#include <vector>

namespace ShootEmUp {
namespace Components {

/**
 * @brief Boss component for multi-phase boss enemies
 *
 * Bosses have multiple phases with different attack patterns,
 * weak points, and behaviors.
 */
struct Boss {
    // Boss identifier
    std::string bossType = "stage1_boss";
    std::string bossName = "Unknown Boss";

    // Phase system
    int currentPhase = 1;
    int maxPhases = 3;

    // Health per phase (percentage thresholds)
    // Phase 2 starts at 66%, Phase 3 at 33%
    std::vector<float> phaseThresholds = {1.0f, 0.66f, 0.33f};

    // Attack patterns per phase
    std::vector<std::string> phasePatterns = {"spread", "laser_sweep", "bullet_hell"};

    // Current attack state
    std::string currentAttack = "";
    float attackTimer = 0.0f;
    float attackCooldown = 2.0f;
    int attacksInPattern = 0;

    // Weak points (for multi-part bosses)
    bool hasWeakPoint = true;
    std::string weakPointLocation = "core";  // "core", "top", "bottom", "all"
    float weakPointMultiplier = 2.0f;        // Extra damage when hit on weak point

    // Movement
    std::string movementPattern = "hover";  // "hover", "sweep", "charge", "teleport"
    float moveSpeed = 100.0f;
    float hoverAmplitude = 50.0f;
    float hoverFrequency = 1.0f;

    // Entry animation
    bool isEntering = true;
    float entryProgress = 0.0f;
    float entryDuration = 3.0f;
    float targetX = 1400.0f;  // Where boss stops after entry

    // Rage mode (when low health)
    bool inRageMode = false;
    float rageThreshold = 0.2f;  // 20% health
    float rageSpeedMultiplier = 1.5f;
    float rageFireRateMultiplier = 2.0f;

    // Score
    int scoreValue = 10000;
    int phaseBonus = 2000;  // Bonus for each phase destroyed quickly

    // Visual effects
    bool showHealthBar = true;
    bool flashOnHit = true;
    float hitFlashTimer = 0.0f;

    // Sound effects
    std::string phaseChangeSound = "boss_phase";
    std::string deathSound = "boss_death";
    std::string entranceSound = "boss_entrance";
};

/**
 * @brief Boss part component for multi-part bosses
 *
 * Some bosses have multiple destructible parts (turrets, shields, etc.)
 */
struct BossPart {
    ECS::Entity parentBoss = 0;
    std::string partType = "turret";  // "turret", "shield", "core", "arm", "tail"

    // Position relative to boss
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float rotation = 0.0f;

    // Part-specific health (separate from main boss)
    int health = 50;
    int maxHealth = 50;
    bool isDestroyed = false;
    bool respawns = false;
    float respawnTime = 10.0f;
    float respawnTimer = 0.0f;

    // Behavior when destroyed
    bool disablesAttack = true;  // Destroying this disables an attack
    std::string disabledAttack = "";
    bool weakensShield = false;  // Destroying this weakens boss defense
    float shieldReduction = 0.25f;

    // This part's attack
    bool canAttack = true;
    std::string attackType = "aimed_shot";
    float fireRate = 1.5f;
    float lastFireTime = 0.0f;
};

}  // namespace Components
}  // namespace ShootEmUp

#endif  // RTYPE_ENGINE_COMPONENTS_BOSS_HPP
