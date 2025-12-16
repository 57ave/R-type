#include <engine/scripting/ComponentBindings.hpp>

namespace Scripting {

// Use ECS namespace for components
using ECS::Transform;
using ECS::Velocity;
using ECS::Sprite;
using ECS::Health;
using ECS::Damage;
using ECS::AIController;
using ECS::Collider;
using ECS::Player;
using ECS::Enemy;
using ECS::Projectile;
using ECS::PowerUp;

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
    
    // Entity management
    lua["Coordinator"]["CreateEntity"] = [coordinator]() {
        return coordinator->CreateEntity();
    };
    
    lua["Coordinator"]["DestroyEntity"] = [coordinator](ECS::Entity entity) {
        coordinator->DestroyEntity(entity);
    };
    
    lua["Coordinator"]["GetLivingEntityCount"] = [coordinator]() {
        return coordinator->GetLivingEntityCount();
    };

    // Component management - Transform
    lua["Coordinator"]["AddTransform"] = [coordinator](ECS::Entity entity, Transform comp) {
        coordinator->AddComponent(entity, comp);
    };
    
    lua["Coordinator"]["GetTransform"] = [coordinator](ECS::Entity entity) -> Transform& {
        return coordinator->GetComponent<Transform>(entity);
    };
    
    lua["Coordinator"]["HasTransform"] = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<Transform>(entity);
    };

    // Component management - Velocity
    lua["Coordinator"]["AddVelocity"] = [coordinator](ECS::Entity entity, Velocity comp) {
        coordinator->AddComponent(entity, comp);
    };
    
    lua["Coordinator"]["GetVelocity"] = [coordinator](ECS::Entity entity) -> Velocity& {
        return coordinator->GetComponent<Velocity>(entity);
    };

    // Component management - Sprite
    lua["Coordinator"]["AddSprite"] = [coordinator](ECS::Entity entity, Sprite comp) {
        coordinator->AddComponent(entity, comp);
    };
    
    lua["Coordinator"]["GetSprite"] = [coordinator](ECS::Entity entity) -> Sprite& {
        return coordinator->GetComponent<Sprite>(entity);
    };

    // Component management - Health
    lua["Coordinator"]["AddHealth"] = [coordinator](ECS::Entity entity, Health comp) {
        coordinator->AddComponent(entity, comp);
    };
    
    lua["Coordinator"]["GetHealth"] = [coordinator](ECS::Entity entity) -> Health& {
        return coordinator->GetComponent<Health>(entity);
    };

    // Component management - AIController
    lua["Coordinator"]["AddAIController"] = [coordinator](ECS::Entity entity, AIController comp) {
        coordinator->AddComponent(entity, comp);
    };
    
    lua["Coordinator"]["GetAIController"] = [coordinator](ECS::Entity entity) -> AIController& {
        return coordinator->GetComponent<AIController>(entity);
    };

    // Component management - Collider
    lua["Coordinator"]["AddCollider"] = [coordinator](ECS::Entity entity, Collider comp) {
        coordinator->AddComponent(entity, comp);
    };
    
    lua["Coordinator"]["GetCollider"] = [coordinator](ECS::Entity entity) -> Collider& {
        return coordinator->GetComponent<Collider>(entity);
    };
}

} // namespace Scripting
