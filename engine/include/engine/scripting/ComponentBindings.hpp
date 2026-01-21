#pragma once

#include <sol/sol.hpp>

#include "../ecs/Components.hpp"
#include "../ecs/Coordinator.hpp"
#include "LuaState.hpp"

namespace Scripting {

/**
 * @brief ComponentBindings - Exposes ECS components to Lua
 *
 * This class provides bindings between C++ ECS components and Lua scripts.
 * Components themselves are defined in engine/ecs/Components.hpp
 *
 * Usage:
 *   sol::state lua;
 *   ComponentBindings::RegisterAll(lua);
 *   ComponentBindings::RegisterCoordinator(lua, &coordinator);
 */
class ComponentBindings {
public:
    static void RegisterAll(sol::state& lua);

    // Register individual component types
    static void RegisterTransform(sol::state& lua);
    static void RegisterVelocity(sol::state& lua);
    static void RegisterSprite(sol::state& lua);
    static void RegisterHealth(sol::state& lua);
    static void RegisterDamage(sol::state& lua);
    static void RegisterAIController(sol::state& lua);
    static void RegisterCollider(sol::state& lua);
    static void RegisterPlayer(sol::state& lua);
    static void RegisterEnemy(sol::state& lua);
    static void RegisterProjectile(sol::state& lua);
    static void RegisterPowerUp(sol::state& lua);

    // Register ECS coordinator bindings
    static void RegisterCoordinator(sol::state& lua, ECS::Coordinator* coordinator);
};

}  // namespace Scripting
