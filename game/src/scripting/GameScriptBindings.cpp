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
        "playerId", sol::property([](PlayerTag& p) { return p.playerId; }, [](PlayerTag& p, int v) { p.playerId = v; })
    );
}

void GameScriptBindings::RegisterEnemy(sol::state& lua) {
    // EnemyTag now uses strings for types (no enum needed)
    lua.new_usertype<EnemyTag>("EnemyTag",
        sol::constructors<EnemyTag()>(),
        "enemyType", sol::property([](EnemyTag& e) { return e.enemyType; }, [](EnemyTag& e, const std::string& v) { e.enemyType = v; }),
        "scoreValue", sol::property([](EnemyTag& e) { return e.scoreValue; }, [](EnemyTag& e, int v) { e.scoreValue = v; }),
        "aiAggressiveness", sol::property([](EnemyTag& e) { return e.aiAggressiveness; }, [](EnemyTag& e, float v) { e.aiAggressiveness = v; })
    );
}

void GameScriptBindings::RegisterProjectile(sol::state& lua) {
    // ProjectileTag now uses strings for types (no enum needed)
    lua.new_usertype<ProjectileTag>("ProjectileTag",
        sol::constructors<ProjectileTag()>(),
        "projectileType", sol::property([](ProjectileTag& p) { return p.projectileType; }, [](ProjectileTag& p, const std::string& v) { p.projectileType = v; }),
        "ownerId", sol::property([](ProjectileTag& p) { return p.ownerId; }, [](ProjectileTag& p, int v) { p.ownerId = v; }),
        "isPlayerProjectile", sol::property([](ProjectileTag& p) { return p.isPlayerProjectile; }, [](ProjectileTag& p, bool v) { p.isPlayerProjectile = v; }),
        "spriteRow", sol::property([](ProjectileTag& p) { return p.spriteRow; }, [](ProjectileTag& p, int v) { p.spriteRow = v; }),
        "spriteCol", sol::property([](ProjectileTag& p) { return p.spriteCol; }, [](ProjectileTag& p, int v) { p.spriteCol = v; }),
        "pierceCount", sol::property([](ProjectileTag& p) { return p.pierceCount; }, [](ProjectileTag& p, int v) { p.pierceCount = v; }),
        "maxPierceCount", sol::property([](ProjectileTag& p) { return p.maxPierceCount; }, [](ProjectileTag& p, int v) { p.maxPierceCount = v; }),
        "chargeLevel", sol::property([](ProjectileTag& p) { return p.chargeLevel; }, [](ProjectileTag& p, int v) { p.chargeLevel = v; })
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
        "type", sol::property([](PowerUp& p) { return p.type; }, [](PowerUp& p, PowerUp::Type v) { p.type = v; }),
        "duration", sol::property([](PowerUp& p) { return p.duration; }, [](PowerUp& p, float v) { p.duration = v; }),
        "value", sol::property([](PowerUp& p) { return p.value; }, [](PowerUp& p, int v) { p.value = v; })
    );
}

void GameScriptBindings::RegisterAIController(sol::state& lua) {
    lua.new_usertype<AIController>("AIController",
        sol::constructors<AIController()>(),
        "pattern", sol::property([](AIController& a) { return a.pattern; }, [](AIController& a, const std::string& v) { a.pattern = v; }),
        "timer", sol::property([](AIController& a) { return a.timer; }, [](AIController& a, float v) { a.timer = v; }),
        "shootTimer", sol::property([](AIController& a) { return a.shootTimer; }, [](AIController& a, float v) { a.shootTimer = v; }),
        "shootInterval", sol::property([](AIController& a) { return a.shootInterval; }, [](AIController& a, float v) { a.shootInterval = v; }),
        "centerX", sol::property([](AIController& a) { return a.centerX; }, [](AIController& a, float v) { a.centerX = v; }),
        "centerY", sol::property([](AIController& a) { return a.centerY; }, [](AIController& a, float v) { a.centerY = v; }),
        "circleRadius", sol::property([](AIController& a) { return a.circleRadius; }, [](AIController& a, float v) { a.circleRadius = v; }),
        "targetY", sol::property([](AIController& a) { return a.targetY; }, [](AIController& a, float v) { a.targetY = v; }),
        "amplitude", sol::property([](AIController& a) { return a.amplitude; }, [](AIController& a, float v) { a.amplitude = v; }),
        "frequency", sol::property([](AIController& a) { return a.frequency; }, [](AIController& a, float v) { a.frequency = v; })
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
