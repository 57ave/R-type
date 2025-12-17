#ifndef RTYPE_ENGINE_COMPONENTS_HEALTH_HPP
#define RTYPE_ENGINE_COMPONENTS_HEALTH_HPP

struct Health {
    int current = 100;
    int max = 100;
    bool invulnerable = false;

    // Death handling
    bool isDead = false;
    bool destroyOnDeath = true;
    std::string deathEffect = "";     // Name of effect to spawn on death
};

#endif // RTYPE_ENGINE_COMPONENTS_HEALTH_HPP
