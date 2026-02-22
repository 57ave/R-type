#ifndef SHOOTEMUP_FACTORIES_ENEMYFACTORY_HPP
#define SHOOTEMUP_FACTORIES_ENEMYFACTORY_HPP

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
#include <components/ShootEmUpTags.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <sol/sol.hpp>

using namespace eng::engine::rendering::sfml;

/**
 * @brief Generic factory for creating enemies from data (Lua configs).
 *
 * All enemy definitions come from external data (Lua tables).
 * The engine does NOT define game-specific enemy types.
 */
class EnemyFactory {
public:
    /**
     * @brief Create an enemy entity from a Lua config table.
     *
     * This is the primary factory method. All enemy properties (sprite,
     * health, movement pattern, weapon, etc.) are read from the Lua table.
     */
    static ECS::Entity CreateEnemyFromLuaConfig(
        ECS::Coordinator& coordinator,
        float x, float y,
        sol::table config,
        std::unordered_map<std::string, eng::engine::rendering::sfml::SFMLTexture*>& textures,
        std::vector<eng::engine::rendering::sfml::SFMLSprite*>& spriteList
    );

private:
    // Helper to create the base sprite
    static SFMLSprite* CreateEnemySprite(
        float x, float y,
        SFMLTexture* texture,
        int spriteX, int spriteY,
        int spriteWidth, int spriteHeight,
        std::vector<SFMLSprite*>& spriteList
    );
};

#endif // SHOOTEMUP_FACTORIES_ENEMYFACTORY_HPP
