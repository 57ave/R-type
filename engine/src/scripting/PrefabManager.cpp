#include <scripting/PrefabManager.hpp>
#include <ecs/Components.hpp>
#include <iostream>

namespace Scripting {

// Use ECS namespace for GENERIC components only
using rtype::engine::ECS::Transform;
using rtype::engine::ECS::Velocity;
using rtype::engine::ECS::Sprite;
using rtype::engine::ECS::Health;
using rtype::engine::ECS::Damage;
using rtype::engine::ECS::Collider;
using rtype::engine::ECS::Tag;

// NOTE: PrefabManager currently only supports generic engine components.
// Game-specific components (Player, Enemy, etc.) should be handled in game code.

bool PrefabManager::LoadPrefab(const std::string& name, const std::string& scriptPath) {
    auto& lua = LuaState::Instance().GetState();
    
    if (!LuaState::Instance().LoadScript(scriptPath)) {
        return false;
    }

    // Expect script to return a table
    sol::optional<sol::table> prefabTable = lua["prefab"];
    if (!prefabTable) {
        std::cerr << "[PrefabManager] Script must return 'prefab' table: " << scriptPath << std::endl;
        return false;
    }

    mPrefabs[name] = prefabTable.value();
    std::cout << "[PrefabManager] Loaded prefab '" << name << "' from: " << scriptPath << std::endl;
    return true;
}

ECS::Entity PrefabManager::CreateEntity(const std::string& prefabName) {
    return CreateEntity(prefabName, sol::table());
}

ECS::Entity PrefabManager::CreateEntity(const std::string& prefabName, sol::table overrides) {
    auto it = mPrefabs.find(prefabName);
    if (it == mPrefabs.end()) {
        std::cerr << "[PrefabManager] Prefab not found: " << prefabName << std::endl;
        return ECS::Entity(-1);
    }

    ECS::Entity entity = mCoordinator->CreateEntity();
    
    // Apply components from prefab
    sol::table prefab = it->second;
    sol::optional<sol::table> components = prefab["components"];
    
    if (components) {
        ApplyComponentsFromTable(entity, components.value());
    }

    // Apply overrides
    if (overrides.valid()) {
        ApplyComponentsFromTable(entity, overrides);
    }

    return entity;
}

void PrefabManager::Clear() {
    mPrefabs.clear();
}

void PrefabManager::ApplyComponentsFromTable(ECS::Entity entity, sol::table components) {
    // Transform
    sol::optional<sol::table> transformTable = components["Transform"];
    if (transformTable) {
        Transform t;
        t.x = transformTable.value()["x"].get_or(0.0f);
        t.y = transformTable.value()["y"].get_or(0.0f);
        t.rotation = transformTable.value()["rotation"].get_or(0.0f);
        mCoordinator->AddComponent(entity, t);
    }

    // Velocity
    sol::optional<sol::table> velocityTable = components["Velocity"];
    if (velocityTable) {
        Velocity v;
        v.dx = velocityTable.value()["dx"].get_or(0.0f);
        v.dy = velocityTable.value()["dy"].get_or(0.0f);
        v.maxSpeed = velocityTable.value()["maxSpeed"].get_or(1000.0f);
        mCoordinator->AddComponent(entity, v);
    }

    // Sprite
    sol::optional<sol::table> spriteTable = components["Sprite"];
    if (spriteTable) {
        Sprite s;
        s.texturePath = spriteTable.value()["texture"].get_or(std::string(""));
        s.width = spriteTable.value()["width"].get_or(32);
        s.height = spriteTable.value()["height"].get_or(32);
        mCoordinator->AddComponent(entity, s);
    }

    // Health
    sol::optional<sol::table> healthTable = components["Health"];
    if (healthTable) {
        Health h;
        h.current = healthTable.value()["current"].get_or(100);
        h.maximum = healthTable.value()["max"].get_or(100);
        mCoordinator->AddComponent(entity, h);
    }

    // Damage
    sol::optional<sol::table> damageTable = components["Damage"];
    if (damageTable) {
        Damage d;
        d.value = damageTable.value()["value"].get_or(10);
        mCoordinator->AddComponent(entity, d);
    }

    // Tag (generic string tag)
    sol::optional<sol::table> tagTable = components["Tag"];
    if (tagTable) {
        Tag t;
        t.value = tagTable.value()["value"].get_or(std::string(""));
        mCoordinator->AddComponent(entity, t);
    }

    // Collider
    sol::optional<sol::table> colliderTable = components["Collider"];
    if (colliderTable) {
        Collider c;
        c.radius = colliderTable.value()["radius"].get_or(16.0f);
        c.isTrigger = colliderTable.value()["isTrigger"].get_or(false);
        mCoordinator->AddComponent(entity, c);
    }

    // NOTE: Game-specific components (AIController, Player, Enemy, etc.)
    // are not handled here. Extend this in your game code if needed.
}

} // namespace Scripting
