#pragma once

#include "LuaState.hpp"
#include "../ecs/Coordinator.hpp"
#include "../ecs/Components.hpp"
#include <sol/sol.hpp>

/**
 * @file ComponentBindings.hpp
 * @brief Generic ECS component bindings for Lua
 * 
 * This provides ONLY bindings for generic engine components.
 * Game-specific components should be bound in the game project.
 * 
 * Design philosophy:
 * - Engine exposes ONLY generic components (Transform, Velocity, etc.)
 * - Games can register their own components using the same pattern
 * - Completely agnostic of any specific game logic
 */

namespace Scripting {

    /**
     * @brief ComponentBindings - Exposes GENERIC ECS components to Lua
     *
     * This class provides bindings ONLY for engine-level generic components.
     * It does NOT know about Player, Enemy, PowerUp, or any game-specific types.
     * 
     * Usage:
     *   sol::state lua;
     *   ComponentBindings::RegisterAll(lua);           // Register generic components
     *   ComponentBindings::RegisterCoordinator(lua, &coordinator);
     * 
     * For game-specific components, create a similar class in your game project.
     */
    class ComponentBindings {
     public:
            /**
             * @brief Register all generic engine components
             * This includes: Transform, Velocity, Sprite, Health, Damage, Collider, Tag
             */
            static void RegisterAll(sol::state& lua);

            // Register individual generic component types
            static void RegisterTransform(sol::state& lua);
            static void RegisterVelocity(sol::state& lua);
            static void RegisterSprite(sol::state& lua);
            static void RegisterHealth(sol::state& lua);
            static void RegisterDamage(sol::state& lua);
            static void RegisterCollider(sol::state& lua);
            static void RegisterTag(sol::state& lua);

            /**
             * @brief Register ECS coordinator bindings for GENERIC components only
             * This provides Lua access to:
             * - CreateEntity / DestroyEntity
             * - AddComponent / GetComponent / HasComponent for generic types
             * 
             * Game-specific component management should be registered separately.
             */
            static void RegisterCoordinator(sol::state& lua, ECS::Coordinator* coordinator);
    };

} // namespace Scripting
