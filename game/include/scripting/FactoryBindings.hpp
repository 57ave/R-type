#pragma once

#include <ecs/Coordinator.hpp>
#include <functional>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <scripting/LuaState.hpp>
#include <sol/sol.hpp>
#include <unordered_map>
#include <vector>

/**
 * @file FactoryBindings.hpp
 * @brief R-Type specific factory bindings for Lua
 *
 * This is GAME-SPECIFIC code that exposes R-Type entity factories to Lua.
 * It belongs in game/, not engine/.
 */

namespace RType {
namespace Scripting {

/**
 * @brief FactoryBindings - Exposes R-Type entity creation factories to Lua
 *
 * This allows Lua scripts to create enemies, projectiles, and other R-Type
 * entities using the existing C++ factory pattern.
 */
class FactoryBindings {
public:
    /**
     * @brief Register R-Type factory functions to Lua state
     *
     * @param lua Sol2 state
     * @param coordinator ECS Coordinator
     * @param textures Map of texture names to texture pointers
     * @param spriteList Reference to sprite cleanup list
     */
    static void RegisterFactories(
        sol::state& lua, ECS::Coordinator* coordinator,
        std::unordered_map<std::string, eng::engine::rendering::sfml::SFMLTexture*> textures,
        std::vector<eng::engine::rendering::sfml::SFMLSprite*>* spriteList,
        std::function<void(ECS::Entity)> registerEntityCallback);

private:
    // Helper to store context in Lua registry
    struct FactoryContext {
        ECS::Coordinator* coordinator;
        std::unordered_map<std::string, eng::engine::rendering::sfml::SFMLTexture*> textures;
        std::vector<eng::engine::rendering::sfml::SFMLSprite*>* spriteList;
        sol::state* lua;
        std::function<void(ECS::Entity)> registerEntity;
    };
};

}  // namespace Scripting
}  // namespace RType
