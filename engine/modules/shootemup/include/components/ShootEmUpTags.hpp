#ifndef SHOOTEMUP_COMPONENTS_TAGS_HPP
#define SHOOTEMUP_COMPONENTS_TAGS_HPP

#include <string>

/**
 * @file ShootEmUpTags.hpp
 * @brief Shoot'em up specific tag components (reusable module)
 * 
 * These are GENERIC shoot'em up tag components that can be used
 * in ANY shoot'em up game.
 */

namespace ShootEmUp {
namespace Components {

    /**
     * @brief PlayerTag - Identifies a player entity
     * Generic for any shoot'em up with multiplayer support
     */
    struct PlayerTag {
        int playerId = 0;
        
        PlayerTag() = default;
        PlayerTag(int id) : playerId(id) {}
    };

    /**
     * @brief EnemyTag - Identifies an enemy entity
     * Generic enemy tagging for any shoot'em up game
     * All enemy types are defined as strings (configured in Lua)
     */
    struct EnemyTag {
        // Enemy type identifier (defined in Lua config)
        // Examples: "basic", "zigzag", "sine_wave", "kamikaze", "turret", "boss", etc.
        std::string enemyType = "basic";
        
        int scoreValue = 100;           // Points when destroyed
        float aiAggressiveness = 1.0f;  // AI aggressiveness multiplier (1.0 = normal)
        
        EnemyTag() = default;
        EnemyTag(const std::string& type, int score = 100) 
            : enemyType(type), scoreValue(score) {}
    };

    /**
     * @brief ProjectileTag - Identifies a projectile entity
     * Generic projectile tagging for any shoot'em up game
     * All projectile types are defined as strings (configured in Lua)
     */
    struct ProjectileTag {
        // Projectile type identifier (defined in Lua config)
        // Examples: "normal", "charged", "explosive", "piercing", "homing", "laser", "wave", etc.
        std::string projectileType = "normal";
        
        int ownerId = 0;                // Entity that shot the projectile
        bool isPlayerProjectile = true;

        // Visual properties (for different sprites based on type)
        int spriteRow = 0;              // Row in spritesheet
        int spriteCol = 0;              // Column in spritesheet

        // Gameplay properties
        int pierceCount = 0;            // Number of enemies already pierced
        int maxPierceCount = 0;         // Maximum enemies to pierce
        int chargeLevel = 0;            // Charge level (0 = normal, 1-5 = charged)
        
        ProjectileTag() = default;
        ProjectileTag(const std::string& type, int owner = 0, bool isPlayer = true) 
            : projectileType(type), ownerId(owner), isPlayerProjectile(isPlayer) {}
    };

} // namespace Components
} // namespace ShootEmUp

// Convenience aliases for easier usage
using ShootEmUpPlayerTag = ShootEmUp::Components::PlayerTag;
using ShootEmUpEnemyTag = ShootEmUp::Components::EnemyTag;
using ShootEmUpProjectileTag = ShootEmUp::Components::ProjectileTag;

#endif // SHOOTEMUP_COMPONENTS_TAGS_HPP
