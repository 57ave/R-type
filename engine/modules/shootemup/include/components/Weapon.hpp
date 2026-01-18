#ifndef RTYPE_ENGINE_COMPONENTS_WEAPON_HPP
#define RTYPE_ENGINE_COMPONENTS_WEAPON_HPP

#include <string>

namespace ShootEmUp {
namespace Components {

/**
 * @brief Generic weapon component for shoot'em up games
 * 
 * All weapon types are defined as strings (configured in Lua).
 * This allows any game to define its own weapon types without modifying engine code.
 */
struct Weapon {
    // Weapon type identifier (defined in Lua config)
    // Examples: "single_shot", "double_shot", "laser", "homing_missile", etc.
    std::string weaponType = "single_shot";
    
    int level = 1;                 // Upgrade level (1-5 or more)
    
    float fireRate = 0.5f;         // Time between shots (seconds)
    float lastFireTime = 0.0f;     // Time since last shot
    bool canFire = true;

    // Charge mechanic
    bool supportsCharge = false;
    float chargeTime = 0.0f;       // Current charge time
    float minChargeTime = 0.1f;    // Minimum time to start charging
    float maxChargeTime = 1.0f;    // Max charge time
    bool isCharging = false;

    // Projectile properties (types defined in Lua)
    std::string projectileType = "normal_bullet";
    float projectileSpeed = 1000.0f;
    int damage = 1;

    // Audio (sound names defined in Lua/config)
    std::string shootSound = "";
    
    // Multi-shot properties
    int projectileCount = 1;       // Number of projectiles per shot
    float spreadAngle = 0.0f;      // Spread angle for multi-shot (degrees)
    
    // Custom properties (can be read from Lua for game-specific mechanics)
    std::string customData = "";   // JSON or custom format for extra data
};

} // namespace Components
} // namespace ShootEmUp

#endif // RTYPE_ENGINE_COMPONENTS_WEAPON_HPP
