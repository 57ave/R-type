#pragma once

#include <scripting/LuaState.hpp>
#include <ecs/Coordinator.hpp>
#include <sol/sol.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <vector>

namespace Scripting {

    /**
     * @brief FactoryBindings - Exposes entity creation factories to Lua
     * 
     * This allows Lua scripts to create enemies, projectiles, and other entities
     * using the existing C++ factory pattern.
     */
    class FactoryBindings {
    public:
        /**
         * @brief Register factory functions to Lua state
         * 
         * @param lua Sol2 state
         * @param coordinator ECS Coordinator
         * @param textures Map of texture names to texture pointers
         * @param spriteList Reference to sprite cleanup list
         */
        static void RegisterFactories(
            sol::state& lua,
            ECS::Coordinator* coordinator,
            std::unordered_map<std::string, rtype::engine::rendering::sfml::SFMLTexture*> textures,
            std::vector<rtype::engine::rendering::sfml::SFMLSprite*>* spriteList
        );

    private:
        // Helper to store context in Lua registry
        struct FactoryContext {
            ECS::Coordinator* coordinator;
            std::unordered_map<std::string, rtype::engine::rendering::sfml::SFMLTexture*> textures;
            std::vector<rtype::engine::rendering::sfml::SFMLSprite*>* spriteList;
        };
    };

} // namespace Scripting
