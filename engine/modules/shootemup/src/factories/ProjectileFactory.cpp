#include "factories/ProjectileFactory.hpp"

#include <rendering/Types.hpp>

#include "components/ShootEmUpTags.hpp"

using namespace eng::engine::rendering;
using namespace ShootEmUp::Components;

// Implementation: Create a projectile from a visual spec (used when visuals come from Lua configs)
ECS::Entity ProjectileFactory::CreateProjectileFromSpec(ECS::Coordinator& coordinator, float x,
                                                        float y, SFMLTexture* texture,
                                                        const ProjectileVisualSpec& spec,
                                                        std::vector<SFMLSprite*>& spriteList,
                                                        bool isPlayerProjectile, int ownerId,
                                                        int /*level*/
) {
    ECS::Entity projectile = coordinator.CreateEntity();

    coordinator.AddComponent(projectile, Position{x, y});
    // Default forward velocity for player, backward for enemy
    float speed = isPlayerProjectile ? 1000.0f : -800.0f;
    coordinator.AddComponent(projectile, Velocity{speed, 0.0f});

    // Create sprite from spec
    auto* sprite =
        CreateProjectileSprite(x, y, texture, spec.x, spec.y, spec.w, spec.h, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(spec.x, spec.y, spec.w, spec.h);
    spriteComp.layer = 8;
    spriteComp.scaleX = spec.scale;
    spriteComp.scaleY = spec.scale;
    coordinator.AddComponent(projectile, spriteComp);

    // Animation if requested
    if (spec.animated && spec.frameCount > 1) {
        Animation anim;
        anim.frameTime = spec.frameTime;
        anim.currentFrame = 0;
        anim.frameCount = spec.frameCount;
        anim.loop = true;
        anim.frameWidth = spec.w;
        anim.frameHeight = spec.h;
        anim.startX = spec.x;
        anim.startY = spec.y;
        anim.spacing = spec.spacing;
        coordinator.AddComponent(projectile, anim);
    }

    // Collider sized from visual size
    Collider collider;
    collider.width = spec.w * spec.scale;
    collider.height = spec.h * spec.scale;
    collider.tag = isPlayerProjectile ? "bullet" : "enemy_bullet";
    coordinator.AddComponent(projectile, collider);

    // Damage: default small for normal projectiles; can be overridden later
    Damage damage;
    damage.amount = 1;
    damage.damageType = "normal";
    coordinator.AddComponent(projectile, damage);

    coordinator.AddComponent(projectile, Tag{isPlayerProjectile ? "bullet" : "enemy_bullet"});

    ProjectileTag projTag;
    projTag.projectileType = "normal";
    projTag.ownerId = ownerId;
    projTag.isPlayerProjectile = isPlayerProjectile;
    coordinator.AddComponent(projectile, projTag);

    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    coordinator.AddComponent(projectile, lifetime);

    return projectile;
}

// Helper pour créer le sprite de base
SFMLSprite* ProjectileFactory::CreateProjectileSprite(float x, float y, SFMLTexture* texture,
                                                      int spriteX, int spriteY, int spriteWidth,
                                                      int spriteHeight,
                                                      std::vector<SFMLSprite*>& spriteList) {
    auto* sprite = new SFMLSprite();
    spriteList.push_back(sprite);
    sprite->setTexture(texture);
    IntRect rect(spriteX, spriteY, spriteWidth, spriteHeight);
    sprite->setTextureRect(rect);
    sprite->setPosition(Vector2f(x, y));
    return sprite;
}

// Projectile NORMAL
ECS::Entity ProjectileFactory::CreateNormalProjectile(ECS::Coordinator& coordinator, float x,
                                                      float y, SFMLTexture* texture,
                                                      std::vector<SFMLSprite*>& spriteList,
                                                      bool isPlayerProjectile, int ownerId) {
    ECS::Entity projectile = coordinator.CreateEntity();

    coordinator.AddComponent(projectile, Position{x, y});

    float speed = isPlayerProjectile ? 1000.0f : -800.0f;
    coordinator.AddComponent(projectile, Velocity{speed, 0.0f});

    auto* sprite = CreateProjectileSprite(x, y, texture, 245, 85, 20, 20, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(245, 85, 20, 20);
    spriteComp.layer = 8;
    coordinator.AddComponent(projectile, spriteComp);

    Collider collider;
    collider.width = 20 * 3.0f;
    collider.height = 20 * 3.0f;
    collider.tag = isPlayerProjectile ? "bullet" : "enemy_bullet";
    coordinator.AddComponent(projectile, collider);

    Damage damage;
    damage.amount = 1;
    damage.damageType = "normal";
    coordinator.AddComponent(projectile, damage);

    coordinator.AddComponent(projectile, Tag{isPlayerProjectile ? "bullet" : "enemy_bullet"});

    ProjectileTag projTag;
    projTag.projectileType = "normal";
    projTag.ownerId = ownerId;
    projTag.isPlayerProjectile = isPlayerProjectile;
    coordinator.AddComponent(projectile, projTag);

    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    coordinator.AddComponent(projectile, lifetime);

    return projectile;
}

// Projectile CHARGED
ECS::Entity ProjectileFactory::CreateChargedProjectile(ECS::Coordinator& coordinator, float x,
                                                       float y, int chargeLevel,
                                                       SFMLTexture* texture,
                                                       std::vector<SFMLSprite*>& spriteList,
                                                       bool isPlayerProjectile, int ownerId) {
    ECS::Entity projectile = coordinator.CreateEntity();

    coordinator.AddComponent(projectile, Position{x, y});
    coordinator.AddComponent(projectile, Velocity{1500.0f, 0.0f});

    // Charged missile sprites (lines 5-9) selon le niveau
    struct ChargeData {
        int xPos, yPos, width, height;
    };
    ChargeData chargeLevels[5] = {
        {233, 100, 15, 15},  // Level 1
        {202, 117, 31, 15},  // Level 2
        {170, 135, 47, 15},  // Level 3
        {138, 155, 63, 15},  // Level 4
        {105, 170, 79, 17}   // Level 5
    };

    int level = chargeLevel > 0 && chargeLevel <= 5 ? chargeLevel - 1 : 0;
    ChargeData& data = chargeLevels[level];

    auto* sprite = CreateProjectileSprite(x, y, texture, data.xPos, data.yPos, data.width,
                                          data.height, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(data.xPos, data.yPos, data.width, data.height);
    spriteComp.layer = 8;
    coordinator.AddComponent(projectile, spriteComp);

    // Animation pour charged missiles
    Animation anim;
    anim.frameTime = 0.1f;
    anim.currentFrame = 0;
    anim.frameCount = 2;
    anim.loop = true;
    anim.frameWidth = data.width;
    anim.frameHeight = data.height;
    anim.startX = data.xPos;
    anim.startY = data.yPos;
    anim.spacing = data.width + 2;
    coordinator.AddComponent(projectile, anim);

    Collider collider;
    collider.width = data.width * 3.0f;
    collider.height = data.height * 3.0f;
    collider.tag = "charged_bullet";
    coordinator.AddComponent(projectile, collider);

    Damage damage;
    damage.amount = chargeLevel;
    damage.damageType = "charged";
    coordinator.AddComponent(projectile, damage);

    coordinator.AddComponent(projectile, Tag{"charged_bullet"});

    ProjectileTag projTag;
    projTag.projectileType = "charged";
    projTag.ownerId = ownerId;
    projTag.isPlayerProjectile = isPlayerProjectile;
    projTag.spriteRow = level;
    coordinator.AddComponent(projectile, projTag);

    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    coordinator.AddComponent(projectile, lifetime);

    return projectile;
}

// Projectile EXPLOSIVE
ECS::Entity ProjectileFactory::CreateExplosiveProjectile(ECS::Coordinator& coordinator, float x,
                                                         float y, SFMLTexture* texture,
                                                         std::vector<SFMLSprite*>& spriteList,
                                                         bool isPlayerProjectile, int ownerId) {
    ECS::Entity projectile = coordinator.CreateEntity();

    coordinator.AddComponent(projectile, Position{x, y});
    coordinator.AddComponent(projectile, Velocity{900.0f, 0.0f});

    auto* sprite = CreateProjectileSprite(x, y, texture, 245, 85, 20, 20, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(245, 85, 20, 20);
    spriteComp.layer = 8;
    spriteComp.scaleX = 1.5f;  // Plus gros
    spriteComp.scaleY = 1.5f;
    coordinator.AddComponent(projectile, spriteComp);

    Collider collider;
    collider.width = 20 * 4.5f;
    collider.height = 20 * 4.5f;
    collider.tag = "explosive_bullet";
    coordinator.AddComponent(projectile, collider);

    Damage damage;
    damage.amount = 3;
    damage.damageType = "explosive";
    damage.explosionRadius = 100.0f;  // Rayon d'explosion
    coordinator.AddComponent(projectile, damage);

    coordinator.AddComponent(projectile, Tag{"explosive_bullet"});

    ProjectileTag projTag;
    projTag.projectileType = "explosive";
    projTag.ownerId = ownerId;
    projTag.isPlayerProjectile = isPlayerProjectile;
    coordinator.AddComponent(projectile, projTag);

    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    coordinator.AddComponent(projectile, lifetime);

    return projectile;
}

// Projectile PIERCING
ECS::Entity ProjectileFactory::CreatePiercingProjectile(ECS::Coordinator& coordinator, float x,
                                                        float y, int maxPierceCount,
                                                        SFMLTexture* texture,
                                                        std::vector<SFMLSprite*>& spriteList,
                                                        bool isPlayerProjectile, int ownerId) {
    ECS::Entity projectile = coordinator.CreateEntity();

    coordinator.AddComponent(projectile, Position{x, y});
    coordinator.AddComponent(projectile, Velocity{1200.0f, 0.0f});

    auto* sprite = CreateProjectileSprite(x, y, texture, 245, 85, 20, 20, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(245, 85, 20, 20);
    spriteComp.layer = 8;
    coordinator.AddComponent(projectile, spriteComp);

    Collider collider;
    collider.width = 20 * 3.0f;
    collider.height = 20 * 3.0f;
    collider.tag = "piercing_bullet";
    coordinator.AddComponent(projectile, collider);

    Damage damage;
    damage.amount = 2;
    damage.damageType = "piercing";
    damage.piercing = true;
    damage.maxPierceCount = maxPierceCount;
    coordinator.AddComponent(projectile, damage);

    coordinator.AddComponent(projectile, Tag{"piercing_bullet"});

    ProjectileTag projTag;
    projTag.projectileType = "piercing";
    projTag.ownerId = ownerId;
    projTag.isPlayerProjectile = isPlayerProjectile;
    projTag.maxPierceCount = maxPierceCount;
    coordinator.AddComponent(projectile, projTag);

    Lifetime lifetime;
    lifetime.maxLifetime = 5.0f;
    coordinator.AddComponent(projectile, lifetime);

    return projectile;
}

// Projectile HOMING
ECS::Entity ProjectileFactory::CreateHomingProjectile(ECS::Coordinator& coordinator, float x,
                                                      float y, SFMLTexture* texture,
                                                      std::vector<SFMLSprite*>& spriteList,
                                                      bool isPlayerProjectile, int ownerId) {
    ECS::Entity projectile = coordinator.CreateEntity();

    coordinator.AddComponent(projectile, Position{x, y});
    coordinator.AddComponent(projectile, Velocity{800.0f, 0.0f});

    auto* sprite = CreateProjectileSprite(x, y, texture, 245, 85, 20, 20, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(245, 85, 20, 20);
    spriteComp.layer = 8;
    coordinator.AddComponent(projectile, spriteComp);

    Collider collider;
    collider.width = 20 * 3.0f;
    collider.height = 20 * 3.0f;
    collider.tag = "homing_bullet";
    coordinator.AddComponent(projectile, collider);

    Damage damage;
    damage.amount = 2;
    damage.damageType = "homing";
    coordinator.AddComponent(projectile, damage);

    coordinator.AddComponent(projectile, Tag{"homing_bullet"});

    ProjectileTag projTag;
    projTag.projectileType = "homing";
    projTag.ownerId = ownerId;
    projTag.isPlayerProjectile = isPlayerProjectile;
    coordinator.AddComponent(projectile, projTag);

    Lifetime lifetime;
    lifetime.maxLifetime = 10.0f;  // Dure plus longtemps
    coordinator.AddComponent(projectile, lifetime);

    // TODO: Ajouter un composant HomingTarget avec un système dédié

    return projectile;
}

// Projectile LASER
ECS::Entity ProjectileFactory::CreateLaserProjectile(ECS::Coordinator& coordinator, float x,
                                                     float y, SFMLTexture* texture,
                                                     std::vector<SFMLSprite*>& spriteList,
                                                     bool isPlayerProjectile, int ownerId) {
    ECS::Entity projectile = coordinator.CreateEntity();

    coordinator.AddComponent(projectile, Position{x, y});
    coordinator.AddComponent(projectile, Velocity{2000.0f, 0.0f});  // Très rapide

    auto* sprite = CreateProjectileSprite(x, y, texture, 245, 85, 20, 20, spriteList);
    Sprite spriteComp;
    spriteComp.sprite = sprite;
    spriteComp.textureRect = IntRect(245, 85, 20, 20);
    spriteComp.layer = 8;
    spriteComp.scaleX = 3.0f;  // Laser allongé
    coordinator.AddComponent(projectile, spriteComp);

    Collider collider;
    collider.width = 20 * 9.0f;  // Hitbox allongée
    collider.height = 20 * 3.0f;
    collider.tag = "laser_bullet";
    coordinator.AddComponent(projectile, collider);

    Damage damage;
    damage.amount = 1;
    damage.damageType = "laser";
    damage.piercing = true;
    damage.maxPierceCount = 999;  // Traverse tout
    coordinator.AddComponent(projectile, damage);

    coordinator.AddComponent(projectile, Tag{"laser_bullet"});

    ProjectileTag projTag;
    projTag.projectileType = "laser";
    projTag.ownerId = ownerId;
    projTag.isPlayerProjectile = isPlayerProjectile;
    projTag.maxPierceCount = 999;
    coordinator.AddComponent(projectile, projTag);

    Lifetime lifetime;
    lifetime.maxLifetime = 2.0f;  // Courte durée
    coordinator.AddComponent(projectile, lifetime);

    return projectile;
}

// Factory générique (string-based)
ECS::Entity ProjectileFactory::CreateProjectile(ECS::Coordinator& coordinator,
                                                const std::string& projectileType, float x, float y,
                                                SFMLTexture* texture,
                                                std::vector<SFMLSprite*>& spriteList,
                                                bool isPlayerProjectile, int ownerId, int level) {
    if (projectileType == "normal") {
        return CreateNormalProjectile(coordinator, x, y, texture, spriteList, isPlayerProjectile,
                                      ownerId);
    } else if (projectileType == "charged") {
        return CreateChargedProjectile(coordinator, x, y, level, texture, spriteList,
                                       isPlayerProjectile, ownerId);
    } else if (projectileType == "explosive") {
        return CreateExplosiveProjectile(coordinator, x, y, texture, spriteList, isPlayerProjectile,
                                         ownerId);
    } else if (projectileType == "piercing") {
        return CreatePiercingProjectile(coordinator, x, y, 3, texture, spriteList,
                                        isPlayerProjectile, ownerId);
    } else if (projectileType == "homing") {
        return CreateHomingProjectile(coordinator, x, y, texture, spriteList, isPlayerProjectile,
                                      ownerId);
    } else if (projectileType == "laser") {
        return CreateLaserProjectile(coordinator, x, y, texture, spriteList, isPlayerProjectile,
                                     ownerId);
    } else {
        // Default to normal if unknown type
        return CreateNormalProjectile(coordinator, x, y, texture, spriteList, isPlayerProjectile,
                                      ownerId);
    }
}
