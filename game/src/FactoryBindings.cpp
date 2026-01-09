#include <FactoryBindings.hpp>
#include <factories/EnemyFactory.hpp>
#include <factories/ProjectileFactory.hpp>
#include <components/MovementPattern.hpp>

namespace Scripting {

void FactoryBindings::RegisterFactories(
    sol::state& lua,
    ECS::Coordinator* coordinator,
    std::unordered_map<std::string, rtype::engine::rendering::sfml::SFMLTexture*> textures,
    std::vector<rtype::engine::rendering::sfml::SFMLSprite*>* spriteList
) {
    // Store context in Lua registry for access in callbacks
    FactoryContext* ctx = new FactoryContext{coordinator, textures, spriteList};
    lua["__factory_context"] = sol::make_light(ctx);

    // Create Factory table in Lua
    lua["Factory"] = lua.create_table();

    // ========================================
    // ENEMY FACTORY BINDINGS
    // ========================================
    
    std::function<ECS::Entity(float, float)> createBasic = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) {
            std::cerr << "[Factory] Enemy texture not found!" << std::endl;
            return 0;
        }
        return EnemyFactory::CreateBasicEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
    };
    lua["Factory"]["CreateBasicEnemy"] = createBasic;

    std::function<ECS::Entity(float, float)> createZigZag = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        return EnemyFactory::CreateZigZagEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
    };
    lua["Factory"]["CreateZigZagEnemy"] = createZigZag;

    std::function<ECS::Entity(float, float)> createSineWave = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        return EnemyFactory::CreateSineWaveEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
    };
    lua["Factory"]["CreateSineWaveEnemy"] = createSineWave;

    std::function<ECS::Entity(float, float)> createKamikaze = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        return EnemyFactory::CreateKamikazeEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
    };
    lua["Factory"]["CreateKamikazeEnemy"] = createKamikaze;

    std::function<ECS::Entity(float, float)> createTurret = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        return EnemyFactory::CreateTurretEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
    };
    lua["Factory"]["CreateTurretEnemy"] = createTurret;

    std::function<ECS::Entity(float, float)> createBoss = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        return EnemyFactory::CreateBossEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
    };
    lua["Factory"]["CreateBossEnemy"] = createBoss;

    // Generic enemy creation by type string
    std::function<ECS::Entity(std::string, float, float)> createEnemy = [ctx](std::string enemyType, float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;

        EnemyTag::Type type;
        if (enemyType == "basic") type = EnemyTag::Type::BASIC;
        else if (enemyType == "zigzag") type = EnemyTag::Type::ZIGZAG;
        else if (enemyType == "sinewave") type = EnemyTag::Type::SINE_WAVE;
        else if (enemyType == "kamikaze") type = EnemyTag::Type::KAMIKAZE;
        else if (enemyType == "turret") type = EnemyTag::Type::TURRET;
        else if (enemyType == "boss") type = EnemyTag::Type::BOSS;
        else {
            std::cerr << "[Factory] Unknown enemy type: " << enemyType << std::endl;
            return 0;
        }

        return EnemyFactory::CreateEnemy(*ctx->coordinator, type, x, y, enemyTexture, *ctx->spriteList);
    };
    lua["Factory"]["CreateEnemy"] = createEnemy;

    // ========================================
    // PROJECTILE FACTORY BINDINGS
    // ========================================

    std::function<ECS::Entity(float, float, bool, int)> createNormalProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        return ProjectileFactory::CreateNormalProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
    };
    lua["Factory"]["CreateNormalProjectile"] = createNormalProj;

    std::function<ECS::Entity(float, float, int, bool, int)> createChargedProj = [ctx](float x, float y, int chargeLevel, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        return ProjectileFactory::CreateChargedProjectile(
            *ctx->coordinator, x, y, chargeLevel, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
    };
    lua["Factory"]["CreateChargedProjectile"] = createChargedProj;

    std::function<ECS::Entity(float, float, bool, int)> createExplosiveProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        return ProjectileFactory::CreateExplosiveProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
    };
    lua["Factory"]["CreateExplosiveProjectile"] = createExplosiveProj;

    std::function<ECS::Entity(float, float, int, bool, int)> createPiercingProj = [ctx](float x, float y, int maxPierce, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        return ProjectileFactory::CreatePiercingProjectile(
            *ctx->coordinator, x, y, maxPierce, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
    };
    lua["Factory"]["CreatePiercingProjectile"] = createPiercingProj;

    std::function<ECS::Entity(float, float, bool, int)> createHomingProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        return ProjectileFactory::CreateHomingProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
    };
    lua["Factory"]["CreateHomingProjectile"] = createHomingProj;

    std::function<ECS::Entity(float, float, bool, int)> createLaserProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        return ProjectileFactory::CreateLaserProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
    };
    lua["Factory"]["CreateLaserProjectile"] = createLaserProj;

    std::cout << "[FactoryBindings] Registered all factory functions to Lua" << std::endl;
}

} // namespace Scripting
