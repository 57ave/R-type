#include <scripting/FactoryBindings.hpp>
#include <factories/EnemyFactory.hpp>
#include <factories/ProjectileFactory.hpp>
#include <components/MovementPattern.hpp>

namespace RType {
namespace Scripting {

void FactoryBindings::RegisterFactories(
    sol::state& lua,
    ECS::Coordinator* coordinator,
    std::unordered_map<std::string, eng::engine::rendering::sfml::SFMLTexture*> textures,
    std::vector<eng::engine::rendering::sfml::SFMLSprite*>* spriteList
    , std::function<void(ECS::Entity)> registerEntityCallback
) {
    // Store context in Lua registry for access in callbacks
    FactoryContext* ctx = new FactoryContext{coordinator, textures, spriteList, &lua, registerEntityCallback};
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
        ECS::Entity e = EnemyFactory::CreateBasicEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateBasicEnemy"] = createBasic;

    std::function<ECS::Entity(float, float)> createZigZag = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        ECS::Entity e = EnemyFactory::CreateZigZagEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateZigZagEnemy"] = createZigZag;

    std::function<ECS::Entity(float, float)> createSineWave = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        ECS::Entity e = EnemyFactory::CreateSineWaveEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateSineWaveEnemy"] = createSineWave;

    std::function<ECS::Entity(float, float)> createKamikaze = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        ECS::Entity e = EnemyFactory::CreateKamikazeEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateKamikazeEnemy"] = createKamikaze;

    std::function<ECS::Entity(float, float)> createTurret = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        ECS::Entity e = EnemyFactory::CreateTurretEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateTurretEnemy"] = createTurret;

    std::function<ECS::Entity(float, float)> createBoss = [ctx](float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;
        ECS::Entity e = EnemyFactory::CreateBossEnemy(*ctx->coordinator, x, y, enemyTexture, *ctx->spriteList);
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateBossEnemy"] = createBoss;

    // Generic enemy creation by type string
    std::function<ECS::Entity(std::string, float, float)> createEnemy = [ctx](std::string enemyType, float x, float y) -> ECS::Entity {
        auto* enemyTexture = ctx->textures["enemy"];
        if (!enemyTexture) return 0;

        ECS::Entity e = EnemyFactory::CreateEnemy(*ctx->coordinator, enemyType, x, y, enemyTexture, *ctx->spriteList);
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateEnemy"] = createEnemy;

    // ========================================
    // PROJECTILE FACTORY BINDINGS
    // ========================================

    std::function<ECS::Entity(float, float, bool, int)> createNormalProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        ECS::Entity e = ProjectileFactory::CreateNormalProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateNormalProjectile"] = createNormalProj;

    std::function<ECS::Entity(float, float, int, bool, int)> createChargedProj = [ctx](float x, float y, int chargeLevel, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        ECS::Entity e = ProjectileFactory::CreateChargedProjectile(
            *ctx->coordinator, x, y, chargeLevel, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateChargedProjectile"] = createChargedProj;

    std::function<ECS::Entity(float, float, bool, int)> createExplosiveProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        ECS::Entity e = ProjectileFactory::CreateExplosiveProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateExplosiveProjectile"] = createExplosiveProj;

    std::function<ECS::Entity(float, float, int, bool, int)> createPiercingProj = [ctx](float x, float y, int maxPierce, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        ECS::Entity e = ProjectileFactory::CreatePiercingProjectile(
            *ctx->coordinator, x, y, maxPierce, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreatePiercingProjectile"] = createPiercingProj;

    std::function<ECS::Entity(float, float, bool, int)> createHomingProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        ECS::Entity e = ProjectileFactory::CreateHomingProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateHomingProjectile"] = createHomingProj;

    std::function<ECS::Entity(float, float, bool, int)> createLaserProj = [ctx](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        auto* missileTexture = ctx->textures["missile"];
        if (!missileTexture) return 0;
        ECS::Entity e = ProjectileFactory::CreateLaserProjectile(
            *ctx->coordinator, x, y, missileTexture, *ctx->spriteList, isPlayer, ownerId
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateLaserProjectile"] = createLaserProj;


    std::function<ECS::Entity(std::string, float, float)> createEnemyFromConfig = 
        [ctx](std::string enemyType, float x, float y) -> ECS::Entity {
        sol::state_view lua(*ctx->lua);
        sol::table enemiesConfig = lua["EnemiesConfig"];
        if (!enemiesConfig.valid()) {
            std::cerr << "[Factory] EnemiesConfig not found!" << std::endl;
            return 0;
        }
        sol::table config = enemiesConfig[enemyType];
        if (!config.valid()) {
            std::cerr << "[Factory] Enemy type not found: " << enemyType << std::endl;
            return 0;
        }
        config["enemyType"] = enemyType;
        // Ensure the texture referenced by the enemy config is loaded into the texture map
        try {
            sol::table spriteTable = config["sprite"];
            if (spriteTable.valid()) {
                std::string texPath = spriteTable["texture"].get_or(std::string());
                if (!texPath.empty()) {
                    // If texture not already loaded, attempt to load it (store by its config path)
                    if (ctx->textures.find(texPath) == ctx->textures.end() || ctx->textures[texPath] == nullptr) {
                        auto* newTex = new eng::engine::rendering::sfml::SFMLTexture();
                        // Try several candidate paths: ASSET_BASE_PATH + game/assets/, ASSET_BASE_PATH + texPath, texPath
                        std::string assetBase = lua["ASSET_BASE_PATH"].get_or(std::string(""));
                        std::vector<std::string> candidates;
                        if (!assetBase.empty()) {
                            // Ensure trailing slash
                            if (assetBase.back() != '/' && assetBase.back() != '\\') assetBase += '/';
                            candidates.push_back(assetBase + "game/assets/" + texPath);
                            candidates.push_back(assetBase + texPath);
                        }
                        candidates.push_back(std::string("game/assets/") + texPath);
                        candidates.push_back(texPath);
                        bool loaded = false;
                        for (auto& p : candidates) {
                            if (newTex->loadFromFile(p)) {
                                std::cout << "[Factory] Loaded dynamic texture for enemy: " << p << std::endl;
                                loaded = true;
                                break;
                            }
                        }
                        if (!loaded) {
                            std::cerr << "[Factory] Failed to load enemy texture: " << texPath << std::endl;
                            delete newTex;
                        } else {
                            ctx->textures[texPath] = newTex;
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[Factory] Exception while loading enemy texture: " << e.what() << std::endl;
        }
        ECS::Entity e = EnemyFactory::CreateEnemyFromLuaConfig(
            *ctx->coordinator, x, y, config, ctx->textures, *ctx->spriteList
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateEnemyFromConfig"] = createEnemyFromConfig;
    std::cout << "[FactoryBindings] Lua config-based enemy creation registered" << std::endl;

    std::cout << "[FactoryBindings] Registered all factory functions to Lua" << std::endl;
}

} // namespace Scripting
} // namespace RType
