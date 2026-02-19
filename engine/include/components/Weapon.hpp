#ifndef RTYPE_COMPONENTS_WEAPON_HPP
#define RTYPE_COMPONENTS_WEAPON_HPP

/**
 * @brief Weapon component for entities that can shoot
 * 
 * Stores weapon properties like fire rate, projectile speed and damage.
 * Used by both player and enemies for shooting mechanics.
 */
struct Weapon {
    float fireRate = 1.0f;             // Time between shots (seconds)
    float timeSinceLastFire = 0.0f;    // Accumulated time since last shot
    
    float projectileSpeed = 400.0f;    // Speed of projectiles
    int projectileDamage = 5;          // Damage dealt by projectiles
};

#endif // RTYPE_COMPONENTS_WEAPON_HPP
