#ifndef RTYPE_ENGINE_COMPONENTS_WEAPON_HPP
#define RTYPE_ENGINE_COMPONENTS_WEAPON_HPP

#include <string>

struct Weapon {
    float fireRate = 0.5f;         // Time between shots (seconds)
    float lastFireTime = 0.0f;     // Time since last shot
    bool canFire = true;

    // Charge mechanic
    bool supportsCharge = false;
    float chargeTime = 0.0f;       // Current charge time
    float minChargeTime = 0.1f;    // Minimum time to start charging
    float maxChargeTime = 1.0f;    // Max charge time
    bool isCharging = false;

    // Projectile properties
    std::string projectileType = "normal_bullet";
    float projectileSpeed = 1000.0f;
    int damage = 1;

    // Audio
    std::string shootSound = "";
};

struct Damage {
    int amount = 1;
    std::string damageType = "normal"; // e.g., "normal", "charged", "explosive"
};

#endif // RTYPE_ENGINE_COMPONENTS_WEAPON_HPP
