#ifndef ENEMY_FACTORY_HPP
#define ENEMY_FACTORY_HPP

#include <ecs/Coordinator.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Tag.hpp>
#include <components/MovementPattern.hpp>
#include <vector>

using namespace rtype::engine::rendering::sfml;

/**
 * @brief Factory pour créer différents types d'ennemis
 * 
 * Centralise la création des ennemis avec leurs configurations spécifiques
 */
class EnemyFactory {
public:
    /**
     * @brief Crée un ennemi BASIC - Simple mouvement horizontal
     */
    static ECS::Entity CreateBasicEnemy(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList
    );

    /**
     * @brief Crée un ennemi ZIGZAG - Mouvement en zigzag
     */
    static ECS::Entity CreateZigZagEnemy(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList
    );

    /**
     * @brief Crée un ennemi SINE_WAVE - Mouvement sinusoïdal
     */
    static ECS::Entity CreateSineWaveEnemy(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList
    );

    /**
     * @brief Crée un ennemi KAMIKAZE - Fonce vers le joueur
     */
    static ECS::Entity CreateKamikazeEnemy(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList
    );

    /**
     * @brief Crée une TURRET - Statique qui tire
     */
    static ECS::Entity CreateTurretEnemy(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList
    );

    /**
     * @brief Crée un BOSS - Ennemi puissant avec patterns complexes
     */
    static ECS::Entity CreateBossEnemy(
        ECS::Coordinator& coordinator,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList
    );

    /**
     * @brief Crée un ennemi générique selon le type
     */
    static ECS::Entity CreateEnemy(
        ECS::Coordinator& coordinator,
        EnemyTag::Type enemyType,
        float x, float y,
        SFMLTexture* texture,
        std::vector<SFMLSprite*>& spriteList
    );

private:
    // Helper pour créer le sprite de base
    static SFMLSprite* CreateEnemySprite(
        float x, float y,
        SFMLTexture* texture,
        int spriteX, int spriteY,
        int spriteWidth, int spriteHeight,
        std::vector<SFMLSprite*>& spriteList
    );
};

#endif // ENEMY_FACTORY_HPP
