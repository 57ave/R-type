#include <scripting/ComponentBindings.hpp>

namespace Scripting {

// Use ECS namespace for components
using rtype::engine::ECS::Transform;
using rtype::engine::ECS::Velocity;
using rtype::engine::ECS::Sprite;
using rtype::engine::ECS::Health;
using rtype::engine::ECS::Damage;
using rtype::engine::ECS::AIController;
using rtype::engine::ECS::Collider;
using rtype::engine::ECS::Player;
using rtype::engine::ECS::Enemy;
using rtype::engine::ECS::Projectile;
using rtype::engine::ECS::PowerUp;

void ComponentBindings::RegisterAll(sol::state& lua) {
    RegisterTransform(lua);
    RegisterVelocity(lua);
    RegisterSprite(lua);
    RegisterHealth(lua);
    RegisterDamage(lua);
    RegisterAIController(lua);
    RegisterCollider(lua);
    RegisterPlayer(lua);
    RegisterEnemy(lua);
    RegisterProjectile(lua);
    RegisterPowerUp(lua);
}

void ComponentBindings::RegisterTransform(sol::state& lua) {
    lua.new_usertype<Transform>("Transform",
        sol::constructors<Transform(), Transform(float, float, float)>(),
        "x", &Transform::x,
        "y", &Transform::y,
        "rotation", &Transform::rotation
    );
}

void ComponentBindings::RegisterVelocity(sol::state& lua) {
    lua.new_usertype<Velocity>("Velocity",
        sol::constructors<Velocity(), Velocity(float, float, float)>(),
        "dx", &Velocity::dx,
        "dy", &Velocity::dy,
        "maxSpeed", &Velocity::maxSpeed
    );
}

void ComponentBindings::RegisterSprite(sol::state& lua) {
    lua.new_usertype<Sprite>("Sprite",
        sol::constructors<Sprite(), Sprite(const std::string&, int, int)>(),
        "texturePath", &Sprite::texturePath,
        "width", &Sprite::width,
        "height", &Sprite::height
    );
}

void ComponentBindings::RegisterHealth(sol::state& lua) {
    lua.new_usertype<Health>("Health",
        sol::constructors<Health(), Health(int, int)>(),
        "current", &Health::current,
        "maximum", &Health::maximum
    );
}

void ComponentBindings::RegisterDamage(sol::state& lua) {
    lua.new_usertype<Damage>("Damage",
        sol::constructors<Damage(), Damage(int)>(),
        "value", &Damage::value
    );
}

void ComponentBindings::RegisterAIController(sol::state& lua) {
    lua.new_usertype<AIController>("AIController",
        sol::constructors<AIController()>(),
        "pattern", &AIController::pattern,
        "timer", &AIController::timer,
        "shootTimer", &AIController::shootTimer,
        "shootInterval", &AIController::shootInterval,
        "centerX", &AIController::centerX,
        "centerY", &AIController::centerY,
        "circleRadius", &AIController::circleRadius,
        "targetY", &AIController::targetY
    );
}

void ComponentBindings::RegisterCollider(sol::state& lua) {
    lua.new_usertype<Collider>("Collider",
        sol::constructors<Collider(), Collider(float, bool)>(),
        "radius", &Collider::radius,
        "isTrigger", &Collider::isTrigger
    );
}

void ComponentBindings::RegisterPlayer(sol::state& lua) {
    lua.new_usertype<Player>("Player",
        sol::constructors<Player(), Player(int)>(),
        "playerID", &Player::playerID,
        "score", &Player::score
    );
}

void ComponentBindings::RegisterEnemy(sol::state& lua) {
    lua.new_usertype<Enemy>("Enemy",
        sol::constructors<Enemy(), Enemy(int)>(),
        "scoreValue", &Enemy::scoreValue
    );
}

void ComponentBindings::RegisterProjectile(sol::state& lua) {
    lua.new_usertype<Projectile>("Projectile",
        sol::constructors<Projectile(), Projectile(int, float)>(),
        "ownerID", &Projectile::ownerID,
        "lifetime", &Projectile::lifetime
    );
}

void ComponentBindings::RegisterPowerUp(sol::state& lua) {
    // Register PowerUp enum
    lua.new_enum("PowerUpType",
        "SPEED_BOOST", PowerUp::SPEED_BOOST,
        "DAMAGE_BOOST", PowerUp::DAMAGE_BOOST,
        "HEALTH_RESTORE", PowerUp::HEALTH_RESTORE,
        "SHIELD", PowerUp::SHIELD,
        "WEAPON_UPGRADE", PowerUp::WEAPON_UPGRADE
    );
    
    lua.new_usertype<PowerUp>("PowerUp",
        sol::constructors<PowerUp(), PowerUp(PowerUp::Type, float, int)>(),
        "type", &PowerUp::type,
        "duration", &PowerUp::duration,
        "value", &PowerUp::value
    );
}

void ComponentBindings::RegisterCoordinator(sol::state& lua, ECS::Coordinator* coordinator) {
    // Register Entity type
    lua.new_usertype<ECS::Entity>("Entity");

    // Create global Coordinator table
    lua["Coordinator"] = lua.create_table();
    
    // Entity management using std::function
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

    // Component management - Transform
    std::function<void(ECS::Entity, Transform)> addTransform = [coordinator](ECS::Entity entity, Transform comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddTransform"] = addTransform;
    
    std::function<Transform&(ECS::Entity)> getTransform = [coordinator](ECS::Entity entity) -> Transform& {
        return coordinator->GetComponent<Transform>(entity);
    };
    lua["Coordinator"]["GetTransform"] = getTransform;
    
    std::function<bool(ECS::Entity)> hasTransform = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Transform>(entity);
    };
    lua["Coordinator"]["HasTransform"] = hasTransform;

    // Component management - Velocity
    std::function<void(ECS::Entity, Velocity)> addVelocity = [coordinator](ECS::Entity entity, Velocity comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddVelocity"] = addVelocity;
    
    std::function<Velocity&(ECS::Entity)> getVelocity = [coordinator](ECS::Entity entity) -> Velocity& {
        return coordinator->GetComponent<Velocity>(entity);
    };
    lua["Coordinator"]["GetVelocity"] = getVelocity;

    // Component management - Sprite
    std::function<void(ECS::Entity, Sprite)> addSprite = [coordinator](ECS::Entity entity, Sprite comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddSprite"] = addSprite;
    
    std::function<Sprite&(ECS::Entity)> getSprite = [coordinator](ECS::Entity entity) -> Sprite& {
        return coordinator->GetComponent<Sprite>(entity);
    };
    lua["Coordinator"]["GetSprite"] = getSprite;

    // Component management - Health
    std::function<void(ECS::Entity, Health)> addHealth = [coordinator](ECS::Entity entity, Health comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddHealth"] = addHealth;
    
    std::function<Health&(ECS::Entity)> getHealth = [coordinator](ECS::Entity entity) -> Health& {
        return coordinator->GetComponent<Health>(entity);
    };
    lua["Coordinator"]["GetHealth"] = getHealth;

    // Component management - AIController
    std::function<void(ECS::Entity, AIController)> addAI = [coordinator](ECS::Entity entity, AIController comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddAIController"] = addAI;
    
    std::function<AIController&(ECS::Entity)> getAI = [coordinator](ECS::Entity entity) -> AIController& {
        return coordinator->GetComponent<AIController>(entity);
    };
    lua["Coordinator"]["GetAIController"] = getAI;

    // Component management - Collider
    std::function<void(ECS::Entity, Collider)> addCollider = [coordinator](ECS::Entity entity, Collider comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddCollider"] = addCollider;
    
    std::function<Collider&(ECS::Entity)> getCollider = [coordinator](ECS::Entity entity) -> Collider& {
        return coordinator->GetComponent<Collider>(entity);
    };
    lua["Coordinator"]["GetCollider"] = getCollider;
}

} // namespace Scripting
