#pragma once

#include <scripting/LuaState.hpp>
#include <ecs/Coordinator.hpp>
#include <components/ShootEmUpTags.hpp>
#include <components/PowerUp.hpp>
#include <components/AIController.hpp>
#include <sol/sol.hpp>

/**
 * @file GameScriptBindings.hpp
 * @brief R-Type specific component bindings for Lua
 * 
 * This provides bindings for R-Type specific components:
 * - Player, Enemy, Projectile, PowerUp, AIController
 * 
 * These are separate from the engine's generic ComponentBindings.
 */

namespace RType {
namespace Scripting {

    /**
     * @brief GameScriptBindings - Exposes R-TYPE SPECIFIC components to Lua
     *
     * This class provides bindings ONLY for R-Type game-specific components.
     * Generic engine components are handled by engine's ComponentBindings.
     * 
     * Usage in Game.cpp:
     *   sol::state lua;
     *   // Register generic engine components first
     *   ::Scripting::ComponentBindings::RegisterAll(lua);
     *   ::Scripting::ComponentBindings::RegisterCoordinator(lua, &coordinator);
     *   
     *   // Then register R-Type specific components
     *   RType::Scripting::GameScriptBindings::RegisterAll(lua);
     *   RType::Scripting::GameScriptBindings::RegisterGameCoordinator(lua, &coordinator);
     */
    class GameScriptBindings {
     public:
            /**
             * @brief Register all R-Type specific components
             * This includes: Player, Enemy, Projectile, PowerUp, AIController
             */
            static void RegisterAll(sol::state& lua);

            // Register individual game component types
            static void RegisterPlayer(sol::state& lua);
            static void RegisterEnemy(sol::state& lua);
            static void RegisterProjectile(sol::state& lua);
            static void RegisterPowerUp(sol::state& lua);
            static void RegisterAIController(sol::state& lua);

            /**
             * @brief Register coordinator bindings for R-Type specific components
             * This extends the Coordinator table with game-specific component management.
             */
            static void RegisterGameCoordinator(sol::state& lua, ECS::Coordinator* coordinator);
    };

} // namespace Scripting
} // namespace RType
