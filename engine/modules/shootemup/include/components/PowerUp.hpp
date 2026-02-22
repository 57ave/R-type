#ifndef SHOOTEMUP_COMPONENTS_POWERUP_HPP
#define SHOOTEMUP_COMPONENTS_POWERUP_HPP

/**
 * @file PowerUp.hpp
 * @brief Generic power-up component for shoot'em up games
 * 
 * Defines common power-up types that can be used in ANY shoot'em up game.
 */

namespace ShootEmUp {
namespace Components {

    /**
     * @brief PowerUp component - Collectible power-ups
     * Generic power-up types found in most shoot'em ups
     */
    struct PowerUp {
        enum Type {
            SPEED_BOOST,        // Temporary or permanent speed increase
            DAMAGE_BOOST,       // Increase weapon damage
            HEALTH_RESTORE,     // Restore player health
            SHIELD,             // Temporary invincibility/shield
            WEAPON_UPGRADE,     // Upgrade weapon level
            MULTI_SHOT,         // Enable multi-directional shooting
            RAPID_FIRE,         // Increase fire rate
            BOMB,               // Screen-clearing bomb
            EXTRA_LIFE          // Extra life/continue
        };

        Type type;
        float duration;     // Duration in seconds (0 = permanent/instant)
        int value;          // Strength/amount of the power-up

        PowerUp() : type(HEALTH_RESTORE), duration(0), value(25) {}
        PowerUp(Type t, float dur = 0, int val = 25) : type(t), duration(dur), value(val) {}
    };

} // namespace Components
} // namespace ShootEmUp

// Convenience alias
using ShootEmUpPowerUp = ShootEmUp::Components::PowerUp;

#endif // SHOOTEMUP_COMPONENTS_POWERUP_HPP
