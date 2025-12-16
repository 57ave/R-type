#ifndef RTYPE_ENGINE_COMPONENTS_LIFETIME_HPP
#define RTYPE_ENGINE_COMPONENTS_LIFETIME_HPP

struct Lifetime {
    float timeAlive = 0.0f;
    float maxLifetime = 5.0f;      // Max time before destruction
    bool destroyOnExpire = true;
};

struct Effect {
    enum class Type {
        SHOOT,          // Shoot muzzle flash
        EXPLOSION,      // Explosion effect
        CHARGE,         // Charge effect
        PARTICLE        // Generic particle effect
    };

    Type effectType = Type::SHOOT;
    bool followParent = false;      // Follow parent entity?
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

#endif // RTYPE_ENGINE_COMPONENTS_LIFETIME_HPP
