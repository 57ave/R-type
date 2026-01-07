#ifndef RTYPE_ENGINE_COMPONENTS_WEAPON_HPP
#define RTYPE_ENGINE_COMPONENTS_WEAPON_HPP

#include <string>

struct Weapon {
    enum class Type {
        SINGLE_SHOT,    // Tir simple standard
        DOUBLE_SHOT,    // Tir double (2 projectiles parallèles)
        TRIPLE_SHOT,    // Tir triple
        SPREAD_SHOT,    // Tir en éventail (3-5 projectiles)
        LASER,          // Laser continu
        HOMING_MISSILE, // Missiles à tête chercheuse
        WAVE_BEAM,      // Faisceau ondulé
        CHARGE_CANNON   // Canon avec charge
    };
    
    Type weaponType = Type::SINGLE_SHOT;
    int level = 1;                 // Niveau d'amélioration (1-5)
    
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
    
    // Multi-shot properties
    int projectileCount = 1;       // Nombre de projectiles par tir
    float spreadAngle = 0.0f;      // Angle d'écartement pour les tirs multiples (en degrés)
};

struct Damage {
    int amount = 1;
    std::string damageType = "normal"; // e.g., "normal", "charged", "explosive"
    
    // Additional properties
    bool piercing = false;         // Le projectile traverse les ennemis
    int maxPierceCount = 0;        // Nombre maximum d'ennemis à traverser
    float explosionRadius = 0.0f;  // Rayon d'explosion (0 = pas d'explosion)
};

#endif // RTYPE_ENGINE_COMPONENTS_WEAPON_HPP
