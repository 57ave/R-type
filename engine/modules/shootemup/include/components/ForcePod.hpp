#ifndef SHOOTEMUP_COMPONENTS_FORCEPOD_HPP
#define SHOOTEMUP_COMPONENTS_FORCEPOD_HPP

#include <ecs/ECS.hpp>
#include <string>

namespace ShootEmUp {
namespace Components {

/**
 * @brief Force Pod component - a force-pod style attachment
 * 
 * The Force is an indestructible pod that can:
 * - Attach to front or back of ship
 * - Be launched as a weapon
 * - Block enemy bullets
 * - Deal contact damage to enemies
 */
struct ForcePod {
    // State
    enum class State {
        Detached,       // Floating freely
        AttachedFront,  // Attached to front of ship
        AttachedBack,   // Attached to back of ship
        Launching,      // Being launched
        Returning       // Returning to player
    };
    
    State state = State::Detached;
    ECS::Entity owner = 0;  // Player entity
    
    // Force level (affects weapon power)
    int level = 1;  // 1-3
    
    // Position when detached
    float floatOffsetX = 0.0f;
    float floatOffsetY = 0.0f;
    
    // Attachment offsets
    float frontOffsetX = 100.0f;
    float frontOffsetY = 0.0f;
    float backOffsetX = -60.0f;
    float backOffsetY = 0.0f;
    
    // Launch properties
    float launchSpeed = 800.0f;
    float returnSpeed = 600.0f;
    float maxLaunchDistance = 600.0f;
    float currentLaunchDistance = 0.0f;
    
    // Combat properties
    int contactDamage = 5;
    bool blocksEnemyBullets = true;
    float hitboxRadius = 40.0f;
    
    // Weapon type when attached (changes based on power-ups)
    std::string weaponType = "force_laser";  // "force_laser", "force_wave", "force_homing"
    float fireRate = 0.3f;
    float lastFireTime = 0.0f;
    
    // Visual
    std::string spriteType = "force_pod";
    int animationFrame = 0;
    float animationTimer = 0.0f;
    bool isGlowing = false;
};

/**
 * @brief Option/Bit component - trailing options that follow player
 * 
 * Similar to Gradius options - they follow player movement with delay
 */
struct Option {
    ECS::Entity owner = 0;
    int optionIndex = 0;  // 0 = first option, 1 = second, etc.
    
    // Following behavior
    float followDelay = 0.3f;  // Seconds of delay
    std::vector<std::pair<float, float>> positionHistory;
    int historyMaxSize = 60;
    
    // Position offset when stationary
    float idleOffsetX = -50.0f;
    float idleOffsetY = 0.0f;
    
    // Formation type
    std::string formation = "trail";  // "trail", "spread", "rotate", "fixed"
    
    // For rotate formation
    float rotationAngle = 0.0f;
    float rotationSpeed = 180.0f;  // degrees per second
    float rotationRadius = 80.0f;
    
    // Combat
    bool canShoot = true;
    bool mirrorsPlayerFire = true;  // Shoots when player shoots
    std::string projectileType = "option_shot";
    float fireRate = 0.3f;
    float lastFireTime = 0.0f;
    
    // Damage reduction (options can take hits for player)
    bool absorbsDamage = false;
    int damageAbsorbed = 0;
    int maxDamageAbsorb = 3;
};

/**
 * @brief Shield component - temporary protective barrier
 */
struct Shield {
    ECS::Entity owner = 0;
    
    // Shield type
    std::string shieldType = "energy";  // "energy", "barrier", "reflect"
    
    // Health/Duration
    int hitPoints = 3;  // Hits before breaking
    float duration = 0.0f;  // 0 = permanent until destroyed
    float currentTime = 0.0f;
    
    // Coverage
    float radius = 60.0f;
    bool fullCoverage = true;  // false = front only
    float arcAngle = 180.0f;   // For partial coverage
    float rotation = 0.0f;
    
    // Behavior
    bool reflectsBullets = false;
    float reflectDamageMultiplier = 1.5f;
    bool flashOnHit = true;
    float flashTimer = 0.0f;
    
    // Visual
    float opacity = 0.7f;
    std::string color = "blue";  // "blue", "green", "gold"
    float pulseSpeed = 2.0f;
    float currentPulse = 0.0f;
};

/**
 * @brief Speed boost component
 */
struct SpeedBoost {
    int level = 0;  // 0-5
    float baseSpeed = 400.0f;
    float speedPerLevel = 80.0f;
    
    float GetCurrentSpeed() const {
        return baseSpeed + (level * speedPerLevel);
    }
    
    // Afterburner (temporary speed boost)
    bool afterburnerActive = false;
    float afterburnerMultiplier = 1.5f;
    float afterburnerDuration = 2.0f;
    float afterburnerTimer = 0.0f;
    float afterburnerCooldown = 5.0f;
    float afterburnerCooldownTimer = 0.0f;
};

} // namespace Components
} // namespace ShootEmUp

#endif // SHOOTEMUP_COMPONENTS_FORCEPOD_HPP
