#ifndef PROJECTILE_FACTORY_HPP
#define PROJECTILE_FACTORY_HPP

#include <ecs/Coordinator.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
#include <components/Collider.hpp>
#include <components/Weapon.hpp>
#include <components/Tag.hpp>
#include <components/Lifetime.hpp>
#include <vector>

using namespace rtype::engine::rendering::sfml;

/**
 * @brief Factory pour créer différents types de projectiles
 * 
 * Centralise la création des missiles/projectiles avec leurs configurations spécifiques
 */
class ProjectileFactory {
public:
    /**
     * @brief Crée un projectile NORMAL - Simple bullet
     */
    static ECS::Entity CreateNormalProjectile(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList,
        bool isPlayerProjectile = true,
        int ownerId = 0
    );

    /**
     * @brief Crée un projectile CHARGED - Plus puissant
     */
    static ECS::Entity CreateChargedProjectile(
        ECS::Coordinator& coordinator,
        float x, float y,
        int chargeLevel,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList,
        bool isPlayerProjectile = true,
        int ownerId = 0
    );

    /**
     * @brief Crée un projectile EXPLOSIVE - Explose en zone
     */
    static ECS::Entity CreateExplosiveProjectile(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList,
        bool isPlayerProjectile = true,
        int ownerId = 0
    );

    /**
     * @brief Crée un projectile PIERCING - Traverse les ennemis
     */
    static ECS::Entity CreatePiercingProjectile(
        ECS::Coordinator& coordinator,
        float x, float y,
        int maxPierceCount,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList,
        bool isPlayerProjectile = true,
        int ownerId = 0
    );

    /**
     * @brief Crée un projectile HOMING - Suit les ennemis (à implémenter avec un système)
     */
    static ECS::Entity CreateHomingProjectile(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList,
        bool isPlayerProjectile = true,
        int ownerId = 0
    );

    /**
     * @brief Crée un projectile LASER - Rayon laser
     */
    static ECS::Entity CreateLaserProjectile(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList,
        bool isPlayerProjectile = true,
        int ownerId = 0
    );

    /**
     * @brief Factory générique qui dispatche selon le type
     */
    static ECS::Entity CreateProjectile(
        ECS::Coordinator& coordinator,
        ProjectileTag::Type projectileType,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList,
        bool isPlayerProjectile = true,
        int ownerId = 0,
        int level = 1
    );

private:
    // Helper pour créer le sprite de base
    static SFMLSprite* CreateProjectileSprite(
        float x, float y,
        SFMLTexture* texture,
        int spriteX, int spriteY,
        int spriteWidth, int spriteHeight,
        std::vector<SFMLSprite*>& spriteList
    );
};

#endif // PROJECTILE_FACTORY_HPP
