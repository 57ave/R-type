#ifndef ENG_ENGINE_COMPONENTS_LIFETIME_HPP
#define ENG_ENGINE_COMPONENTS_LIFETIME_HPP

/**
 * @brief Generic lifetime component for temporary entities
 * 
 * Useful for: projectiles, particles, temporary effects, etc.
 */
struct Lifetime {
    float timeAlive = 0.0f;
    float maxLifetime = 5.0f;      // Max time before destruction
    bool destroyOnExpire = true;
};

#endif // ENG_ENGINE_COMPONENTS_LIFETIME_HPP
