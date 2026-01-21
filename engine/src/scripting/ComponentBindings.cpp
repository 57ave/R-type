#include <scripting/ComponentBindings.hpp>

namespace Scripting {

// Use ECS namespace for GENERIC components only
using eng::engine::ECS::Collider;
using eng::engine::ECS::Damage;
using eng::engine::ECS::Health;
using eng::engine::ECS::Sprite;
using eng::engine::ECS::Tag;
using eng::engine::ECS::Transform;
using eng::engine::ECS::Velocity;

void ComponentBindings::RegisterAll(sol::state& lua) {
    RegisterTransform(lua);
    RegisterVelocity(lua);
    RegisterSprite(lua);
    RegisterHealth(lua);
    RegisterDamage(lua);
    RegisterCollider(lua);
    RegisterTag(lua);
}

void ComponentBindings::RegisterTransform(sol::state& lua) {
    lua.new_usertype<Transform>(
        "Transform", sol::constructors<Transform(), Transform(float, float, float)>(), "x",
        sol::property([](Transform& t) { return t.x; }, [](Transform& t, float v) { t.x = v; }),
        "y",
        sol::property([](Transform& t) { return t.y; }, [](Transform& t, float v) { t.y = v; }),
        "rotation",
        sol::property([](Transform& t) { return t.rotation; },
                      [](Transform& t, float v) { t.rotation = v; }));
}

void ComponentBindings::RegisterVelocity(sol::state& lua) {
    lua.new_usertype<Velocity>(
        "Velocity", sol::constructors<Velocity(), Velocity(float, float, float)>(), "dx",
        sol::property([](Velocity& v) { return v.dx; }, [](Velocity& v, float val) { v.dx = val; }),
        "dy",
        sol::property([](Velocity& v) { return v.dy; }, [](Velocity& v, float val) { v.dy = val; }),
        "maxSpeed",
        sol::property([](Velocity& v) { return v.maxSpeed; },
                      [](Velocity& v, float val) { v.maxSpeed = val; }));
}

void ComponentBindings::RegisterSprite(sol::state& lua) {
    lua.new_usertype<Sprite>(
        "Sprite", sol::constructors<Sprite(), Sprite(const std::string&, int, int)>(),
        "texturePath",
        sol::property([](Sprite& s) { return s.texturePath; },
                      [](Sprite& s, const std::string& v) { s.texturePath = v; }),
        "width",
        sol::property([](Sprite& s) { return s.width; }, [](Sprite& s, int v) { s.width = v; }),
        "height",
        sol::property([](Sprite& s) { return s.height; }, [](Sprite& s, int v) { s.height = v; }),
        "layer",
        sol::property([](Sprite& s) { return s.layer; }, [](Sprite& s, int v) { s.layer = v; }),
        "visible",
        sol::property([](Sprite& s) { return s.visible; },
                      [](Sprite& s, bool v) { s.visible = v; }));
}

void ComponentBindings::RegisterHealth(sol::state& lua) {
    lua.new_usertype<Health>(
        "Health", sol::constructors<Health(), Health(int, int)>(), "current",
        sol::property([](Health& h) { return h.current; }, [](Health& h, int v) { h.current = v; }),
        "maximum",
        sol::property([](Health& h) { return h.maximum; }, [](Health& h, int v) { h.maximum = v; }),
        "IsAlive", &Health::IsAlive, "TakeDamage", &Health::TakeDamage, "Heal", &Health::Heal);
}

void ComponentBindings::RegisterDamage(sol::state& lua) {
    lua.new_usertype<Damage>(
        "Damage", sol::constructors<Damage(), Damage(int)>(), "value",
        sol::property([](Damage& d) { return d.value; }, [](Damage& d, int v) { d.value = v; }));
}

void ComponentBindings::RegisterCollider(sol::state& lua) {
    lua.new_usertype<Collider>("Collider", sol::constructors<Collider(), Collider(float, bool)>(),
                               "radius",
                               sol::property([](Collider& c) { return c.radius; },
                                             [](Collider& c, float v) { c.radius = v; }),
                               "isTrigger",
                               sol::property([](Collider& c) { return c.isTrigger; },
                                             [](Collider& c, bool v) { c.isTrigger = v; }));
}

void ComponentBindings::RegisterTag(sol::state& lua) {
    lua.new_usertype<Tag>("Tag", sol::constructors<Tag(), Tag(const std::string&)>(), "value",
                          sol::property([](Tag& t) { return t.value; },
                                        [](Tag& t, const std::string& v) { t.value = v; }));
}

void ComponentBindings::RegisterCoordinator(sol::state& lua, ECS::Coordinator* coordinator) {
    // Register Entity type
    lua.new_usertype<ECS::Entity>("Entity");

    // Create global Coordinator table
    lua["Coordinator"] = lua.create_table();

    // ===== Entity management =====
    std::function<ECS::Entity()> createEntity = [coordinator]() {
        return coordinator->CreateEntity();
    };
    lua.set_function("CreateEntity", createEntity);
    lua["Coordinator"]["CreateEntity"] = createEntity;

    std::function<void(ECS::Entity)> destroyEntity = [coordinator](ECS::Entity entity) {
        coordinator->DestroyEntity(entity);
    };
    lua["Coordinator"]["DestroyEntity"] = destroyEntity;

    std::function<uint32_t()> getLivingCount = [coordinator]() {
        return coordinator->GetLivingEntityCount();
    };
    lua["Coordinator"]["GetLivingEntityCount"] = getLivingCount;

    // ===== Transform Component =====
    std::function<void(ECS::Entity, Transform)> addTransform = [coordinator](ECS::Entity entity,
                                                                             Transform comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddTransform"] = addTransform;

    std::function<Transform&(ECS::Entity)> getTransform =
        [coordinator](ECS::Entity entity) -> Transform& {
        return coordinator->GetComponent<Transform>(entity);
    };
    lua["Coordinator"]["GetTransform"] = getTransform;

    std::function<bool(ECS::Entity)> hasTransform = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Transform>(entity);
    };
    lua["Coordinator"]["HasTransform"] = hasTransform;

    // ===== Velocity Component =====
    std::function<void(ECS::Entity, Velocity)> addVelocity = [coordinator](ECS::Entity entity,
                                                                           Velocity comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddVelocity"] = addVelocity;

    std::function<Velocity&(ECS::Entity)> getVelocity =
        [coordinator](ECS::Entity entity) -> Velocity& {
        return coordinator->GetComponent<Velocity>(entity);
    };
    lua["Coordinator"]["GetVelocity"] = getVelocity;

    std::function<bool(ECS::Entity)> hasVelocity = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Velocity>(entity);
    };
    lua["Coordinator"]["HasVelocity"] = hasVelocity;

    // ===== Sprite Component =====
    std::function<void(ECS::Entity, Sprite)> addSprite =
        [coordinator](ECS::Entity entity, Sprite comp) { coordinator->AddComponent(entity, comp); };
    lua["Coordinator"]["AddSprite"] = addSprite;

    std::function<Sprite&(ECS::Entity)> getSprite = [coordinator](ECS::Entity entity) -> Sprite& {
        return coordinator->GetComponent<Sprite>(entity);
    };
    lua["Coordinator"]["GetSprite"] = getSprite;

    std::function<bool(ECS::Entity)> hasSprite = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Sprite>(entity);
    };
    lua["Coordinator"]["HasSprite"] = hasSprite;

    // ===== Health Component =====
    std::function<void(ECS::Entity, Health)> addHealth =
        [coordinator](ECS::Entity entity, Health comp) { coordinator->AddComponent(entity, comp); };
    lua["Coordinator"]["AddHealth"] = addHealth;

    std::function<Health&(ECS::Entity)> getHealth = [coordinator](ECS::Entity entity) -> Health& {
        return coordinator->GetComponent<Health>(entity);
    };
    lua["Coordinator"]["GetHealth"] = getHealth;

    std::function<bool(ECS::Entity)> hasHealth = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Health>(entity);
    };
    lua["Coordinator"]["HasHealth"] = hasHealth;

    // ===== Damage Component =====
    std::function<void(ECS::Entity, Damage)> addDamage =
        [coordinator](ECS::Entity entity, Damage comp) { coordinator->AddComponent(entity, comp); };
    lua["Coordinator"]["AddDamage"] = addDamage;

    std::function<Damage&(ECS::Entity)> getDamage = [coordinator](ECS::Entity entity) -> Damage& {
        return coordinator->GetComponent<Damage>(entity);
    };
    lua["Coordinator"]["GetDamage"] = getDamage;

    std::function<bool(ECS::Entity)> hasDamage = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Damage>(entity);
    };
    lua["Coordinator"]["HasDamage"] = hasDamage;

    // ===== Collider Component =====
    std::function<void(ECS::Entity, Collider)> addCollider = [coordinator](ECS::Entity entity,
                                                                           Collider comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddCollider"] = addCollider;

    std::function<Collider&(ECS::Entity)> getCollider =
        [coordinator](ECS::Entity entity) -> Collider& {
        return coordinator->GetComponent<Collider>(entity);
    };
    lua["Coordinator"]["GetCollider"] = getCollider;

    std::function<bool(ECS::Entity)> hasCollider = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Collider>(entity);
    };
    lua["Coordinator"]["HasCollider"] = hasCollider;

    // ===== Tag Component =====
    std::function<void(ECS::Entity, Tag)> addTag = [coordinator](ECS::Entity entity, Tag comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddTag"] = addTag;

    std::function<Tag&(ECS::Entity)> getTag = [coordinator](ECS::Entity entity) -> Tag& {
        return coordinator->GetComponent<Tag>(entity);
    };
    lua["Coordinator"]["GetTag"] = getTag;

    std::function<bool(ECS::Entity)> hasTag = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Tag>(entity);
    };
    lua["Coordinator"]["HasTag"] = hasTag;
}

}  // namespace Scripting
