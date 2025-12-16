#ifndef RTYPE_ENGINE_COMPONENTS_TAG_HPP
#define RTYPE_ENGINE_COMPONENTS_TAG_HPP

#include <string>

struct Tag {
    std::string name = "entity";

    // Common tags:
    // "player", "enemy", "bullet", "charged_bullet",
    // "effect", "background", "obstacle"
};

struct PlayerTag {
    int playerId = 0;
};

struct EnemyTag {
    std::string enemyType = "basic";
};

struct ProjectileTag {
    int ownerId = 0;               // Entity that shot the projectile
    bool isPlayerProjectile = true;
};

#endif // RTYPE_ENGINE_COMPONENTS_TAG_HPP
