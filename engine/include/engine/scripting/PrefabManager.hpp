#pragma once

#include <sol/sol.hpp>
#include <string>
#include <unordered_map>

#include "../ecs/Coordinator.hpp"
#include "ComponentBindings.hpp"
#include "LuaState.hpp"

namespace Scripting {

// Manages entity prefabs defined in Lua
class PrefabManager {
public:
    PrefabManager(ECS::Coordinator* coordinator) : mCoordinator(coordinator) {}

    // Load prefab from Lua file
    bool LoadPrefab(const std::string& name, const std::string& scriptPath);

    // Create entity from prefab
    ECS::Entity CreateEntity(const std::string& prefabName);

    // Create entity with custom overrides
    ECS::Entity CreateEntity(const std::string& prefabName, sol::table overrides);

    void Clear();

private:
    ECS::Coordinator* mCoordinator;
    std::unordered_map<std::string, sol::table> mPrefabs;

    void ApplyComponentsFromTable(ECS::Entity entity, sol::table components);
};

}  // namespace Scripting
