#ifndef ENG_ENGINE_COMPONENTS_DAMAGE_HPP
#define ENG_ENGINE_COMPONENTS_DAMAGE_HPP

#include <string>

/**
 * @brief Generic damage component for entities that can deal damage
 *
 * Used by: projectiles, melee attacks, traps, hazards, etc.
 * Works for any game genre (shoot'em up, RPG, platformer, etc.)
 */
struct Damage {
    int amount = 1;                     // Damage value
    std::string damageType = "normal";  // Damage type (e.g., "normal", "fire", "ice", "explosive")

    // Optional advanced properties
    bool piercing = false;         // Ignores armor/defense
    int maxPierceCount = 0;        // Number of entities it can pierce through
    float knockback = 0.0f;        // Knockback force
    float explosionRadius = 0.0f;  // Area of effect (0 = no AOE)

    Damage() = default;
    Damage(int dmg) : amount(dmg) {}
    Damage(int dmg, const std::string& type) : amount(dmg), damageType(type) {}
};

#endif  // ENG_ENGINE_COMPONENTS_DAMAGE_HPP
