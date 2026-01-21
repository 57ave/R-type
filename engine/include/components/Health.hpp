#ifndef ENG_ENGINE_COMPONENTS_HEALTH_HPP
#define ENG_ENGINE_COMPONENTS_HEALTH_HPP

#include <string>

struct Health {
    int current = 100;
    int max = 100;
    bool invulnerable = false;

    // Temporary invincibility (after being hit)
    float invincibilityTimer = 0.0f;     // Time remaining of invincibility
    float invincibilityDuration = 0.5f;  // How long invincibility lasts
    bool isFlashing = false;             // Currently in hit flash state
    float flashTimer = 0.0f;             // Timer for flash effect

    // Death handling
    bool isDead = false;
    bool destroyOnDeath = true;
    std::string deathEffect = "";  // Name of effect to spawn on death
};

#endif  // ENG_ENGINE_COMPONENTS_HEALTH_HPP
