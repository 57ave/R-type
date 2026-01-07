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
    enum class Type {
        BASIC,          // Ennemi simple avec mouvement horizontal
        ZIGZAG,         // Ennemi avec mouvement en zigzag
        SINE_WAVE,      // Ennemi avec mouvement sinusoïdal
        KAMIKAZE,       // Ennemi qui fonce sur le joueur
        TURRET,         // Tourelle statique qui tire
        BOSS            // Boss avec patterns complexes
    };

    Type type = Type::BASIC;
    int scoreValue = 100;           // Points gagnés quand détruit
    float aiAggressiveness = 1.0f;  // Multiplicateur d'agressivité (1.0 = normal)

    // Backward compatibility - sera retiré plus tard
    std::string enemyType = "basic";
};

struct ProjectileTag {
    enum class Type {
        NORMAL,         // Projectile normal simple
        CHARGED,        // Projectile chargé avec plus de dégâts
        EXPLOSIVE,      // Projectile qui explose en zone
        PIERCING,       // Projectile qui traverse les ennemis
        HOMING,         // Projectile à tête chercheuse
        LASER,          // Rayon laser continu
        WAVE            // Projectile avec effet d'onde
    };

    Type type = Type::NORMAL;
    int ownerId = 0;                // Entity that shot the projectile
    bool isPlayerProjectile = true;

    // Visual properties (pour différents sprites selon le type)
    int spriteRow = 0;              // Ligne dans la spritesheet
    int spriteCol = 0;              // Colonne dans la spritesheet

    // Gameplay properties
    int pierceCount = 0;            // Nombre d'ennemis déjà traversés (pour PIERCING)
    int maxPierceCount = 0;         // Maximum d'ennemis à traverser
    int chargeLevel = 0;            // Charge level (0 = normal, 1-5 = charged)
};

#endif // RTYPE_ENGINE_COMPONENTS_TAG_HPP
