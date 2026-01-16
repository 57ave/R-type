#include <scripting/GameScriptBindings.hpp>

namespace RType {
namespace Scripting {

// Use shoot'em up module components
using ShootEmUp::Components::PlayerTag;
using ShootEmUp::Components::EnemyTag;
using ShootEmUp::Components::ProjectileTag;
using ShootEmUp::Components::PowerUp;
using ShootEmUp::Components::AIController;

void GameScriptBindings::RegisterAll(sol::state& lua) {
    RegisterPlayer(lua);
    RegisterEnemy(lua);
    RegisterProjectile(lua);
    RegisterPowerUp(lua);
    RegisterAIController(lua);
}

void GameScriptBindings::RegisterPlayer(sol::state& lua) {
    lua.new_usertype<PlayerTag>("PlayerTag",
        sol::constructors<PlayerTag(), PlayerTag(int)>(),
        "playerId", &PlayerTag::playerId
    );
}

void GameScriptBindings::RegisterEnemy(sol::state& lua) {
    // EnemyTag now uses strings for types (no enum needed)
    lua.new_usertype<EnemyTag>("EnemyTag",
        sol::constructors<EnemyTag()>(),
        "enemyType", &EnemyTag::enemyType,  // string-based type
        "scoreValue", &EnemyTag::scoreValue,
        "aiAggressiveness", &EnemyTag::aiAggressiveness
    );
}

void GameScriptBindings::RegisterProjectile(sol::state& lua) {
    // ProjectileTag now uses strings for types (no enum needed)
    lua.new_usertype<ProjectileTag>("ProjectileTag",
        sol::constructors<ProjectileTag()>(),
        "projectileType", &ProjectileTag::projectileType,  // string-based type
        "ownerId", &ProjectileTag::ownerId,
        "isPlayerProjectile", &ProjectileTag::isPlayerProjectile,
        "spriteRow", &ProjectileTag::spriteRow,
        "spriteCol", &ProjectileTag::spriteCol,
        "pierceCount", &ProjectileTag::pierceCount,
        "maxPierceCount", &ProjectileTag::maxPierceCount,
        "chargeLevel", &ProjectileTag::chargeLevel
    );
}

void GameScriptBindings::RegisterPowerUp(sol::state& lua) {
    // Register PowerUp enum
    lua.new_enum("PowerUpType",
        "SPEED_BOOST", PowerUp::SPEED_BOOST,
        "DAMAGE_BOOST", PowerUp::DAMAGE_BOOST,
        "HEALTH_RESTORE", PowerUp::HEALTH_RESTORE,
        "SHIELD", PowerUp::SHIELD,
        "WEAPON_UPGRADE", PowerUp::WEAPON_UPGRADE,
        "MULTI_SHOT", PowerUp::MULTI_SHOT,
        "RAPID_FIRE", PowerUp::RAPID_FIRE,
        "BOMB", PowerUp::BOMB,
        "EXTRA_LIFE", PowerUp::EXTRA_LIFE
    );
    
    lua.new_usertype<PowerUp>("PowerUp",
        sol::constructors<PowerUp(), PowerUp(PowerUp::Type, float, int)>(),
        "type", &PowerUp::type,
        "duration", &PowerUp::duration,
        "value", &PowerUp::value
    );
}

void GameScriptBindings::RegisterAIController(sol::state& lua) {
    lua.new_usertype<AIController>("AIController",
        sol::constructors<AIController()>(),
        "pattern", &AIController::pattern,
        "timer", &AIController::timer,
        "shootTimer", &AIController::shootTimer,
        "shootInterval", &AIController::shootInterval,
        "centerX", &AIController::centerX,
        "centerY", &AIController::centerY,
        "circleRadius", &AIController::circleRadius,
        "targetY", &AIController::targetY,
        "amplitude", &AIController::amplitude,
        "frequency", &AIController::frequency
    );
}

void GameScriptBindings::RegisterGameCoordinator(sol::state& lua, ECS::Coordinator* coordinator) {
    // Coordinator table should already exist from engine's ComponentBindings
    // We just extend it with shoot'em up specific component management
    
    // ===== PlayerTag Component =====
    std::function<void(ECS::Entity, PlayerTag)> addPlayerTag = [coordinator](ECS::Entity entity, PlayerTag comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddPlayerTag"] = addPlayerTag;
    
    std::function<PlayerTag&(ECS::Entity)> getPlayerTag = [coordinator](ECS::Entity entity) -> PlayerTag& {
        return coordinator->GetComponent<PlayerTag>(entity);
    };
    lua["Coordinator"]["GetPlayerTag"] = getPlayerTag;
    
    std::function<bool(ECS::Entity)> hasPlayerTag = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<PlayerTag>(entity);
    };
    lua["Coordinator"]["HasPlayerTag"] = hasPlayerTag;

    // ===== EnemyTag Component =====
    std::function<void(ECS::Entity, EnemyTag)> addEnemyTag = [coordinator](ECS::Entity entity, EnemyTag comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddEnemyTag"] = addEnemyTag;
    
    std::function<EnemyTag&(ECS::Entity)> getEnemyTag = [coordinator](ECS::Entity entity) -> EnemyTag& {
        return coordinator->GetComponent<EnemyTag>(entity);
    };
    lua["Coordinator"]["GetEnemyTag"] = getEnemyTag;
    
    std::function<bool(ECS::Entity)> hasEnemyTag = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<EnemyTag>(entity);
    };
    lua["Coordinator"]["HasEnemyTag"] = hasEnemyTag;

    // ===== ProjectileTag Component =====
    std::function<void(ECS::Entity, ProjectileTag)> addProjectileTag = [coordinator](ECS::Entity entity, ProjectileTag comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddProjectileTag"] = addProjectileTag;
    
    std::function<ProjectileTag&(ECS::Entity)> getProjectileTag = [coordinator](ECS::Entity entity) -> ProjectileTag& {
        return coordinator->GetComponent<ProjectileTag>(entity);
    };
    lua["Coordinator"]["GetProjectileTag"] = getProjectileTag;
    
    std::function<bool(ECS::Entity)> hasProjectileTag = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<ProjectileTag>(entity);
    };
    lua["Coordinator"]["HasProjectileTag"] = hasProjectileTag;

    // ===== PowerUp Component =====
    std::function<void(ECS::Entity, PowerUp)> addPowerUp = [coordinator](ECS::Entity entity, PowerUp comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddPowerUp"] = addPowerUp;
    
    std::function<PowerUp&(ECS::Entity)> getPowerUp = [coordinator](ECS::Entity entity) -> PowerUp& {
        return coordinator->GetComponent<PowerUp>(entity);
    };
    lua["Coordinator"]["GetPowerUp"] = getPowerUp;
    
    std::function<bool(ECS::Entity)> hasPowerUp = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<PowerUp>(entity);
    };
    lua["Coordinator"]["HasPowerUp"] = hasPowerUp;

    // ===== AIController Component =====
    std::function<void(ECS::Entity, AIController)> addAI = [coordinator](ECS::Entity entity, AIController comp) {
        coordinator->AddComponent(entity, comp);
    };
    lua["Coordinator"]["AddAIController"] = addAI;
    
    std::function<AIController&(ECS::Entity)> getAI = [coordinator](ECS::Entity entity) -> AIController& {
        return coordinator->GetComponent<AIController>(entity);
    };
    lua["Coordinator"]["GetAIController"] = getAI;
    
    std::function<bool(ECS::Entity)> hasAI = [coordinator](ECS::Entity entity) {
        return coordinator->HasComponent<AIController>(entity);
    };
    lua["Coordinator"]["HasAIController"] = hasAI;
}

} // namespace Scripting
} // namespace RType
