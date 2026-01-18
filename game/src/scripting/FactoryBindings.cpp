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

    // Helper lambda pour créer des projectiles depuis WeaponsConfig
    auto CreateProjectileFromWeaponConfig = [ctx](
        std::string weaponName,
        float x, float y,
        bool isPlayer,
        int ownerId,
        int level = 1,
        bool charged = false
    ) -> ECS::Entity {
        sol::state_view lua(*ctx->lua);
        sol::table weapons = lua["WeaponsConfig"];
        
        if (!weapons.valid()) {
            std::cerr << "[Factory] WeaponsConfig not found!" << std::endl;
            return 0;
        }
        
        sol::table weaponConfig = weapons[weaponName];
        if (!weaponConfig.valid()) {
            std::cerr << "[Factory] Weapon '" << weaponName << "' not found!" << std::endl;
            return 0;
        }
        
        sol::table projConfig = weaponConfig["projectile"];
        if (!projConfig.valid()) {
            std::cerr << "[Factory] No projectile config for: " << weaponName << std::endl;
            return 0;
        }
        
        // Build spec from config
        ProjectileFactory::ProjectileVisualSpec spec;
        
    // Choose rect (charged or normal)
    sol::table rect;
    if (charged)
        rect = projConfig["chargedRect"];
    else
        rect = projConfig["normalRect"];
    if (!rect.valid())
        rect = projConfig["normalRect"];
        
        if (rect.valid()) {
            spec.x = rect.get_or("x", 0);
            spec.y = rect.get_or("y", 0);
            spec.w = rect.get_or("w", 16);
            spec.h = rect.get_or("h", 16);
        } else {
            spec.x = 0; spec.y = 0; spec.w = 16; spec.h = 16;
        }
        
        spec.scale = projConfig.get_or("scale", 1.0f);
        spec.animated = projConfig.get_or("animated", false);
        spec.frameCount = projConfig.get_or("frameCount", 1);
        spec.frameTime = projConfig.get_or("frameTime", 0.1f);
        spec.spacing = projConfig.get_or("spacing", spec.w);
        
        // Get texture
        std::string texPath;
        try {
            sol::object texObj = projConfig["texture"];
            if (texObj.valid() && texObj.is<std::string>()) {
                texPath = texObj.as<std::string>();
            }
        } catch (...) {
            texPath = "";
        }
        
        eng::engine::rendering::sfml::SFMLTexture* tex = nullptr;
        if (!texPath.empty()) {
            auto it = ctx->textures.find(texPath);
            if (it != ctx->textures.end()) tex = it->second;
        }
        if (!tex) tex = ctx->textures["missile"];
        if (!tex) return 0;
        
        // Create projectile
        ECS::Entity e = ProjectileFactory::CreateProjectileFromSpec(
            *ctx->coordinator, x, y, tex, spec, *ctx->spriteList, isPlayer, ownerId, level
        );
        
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };

    // CreateNormalProjectile - Utilise single_shot ou enemy_bullet
    std::function<ECS::Entity(float, float, bool, int)> createNormalProj = 
        [ctx, CreateProjectileFromWeaponConfig](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        std::string weaponName = isPlayer ? "single_shot" : "enemy_bullet";
        return CreateProjectileFromWeaponConfig(weaponName, x, y, isPlayer, ownerId, 1, false);
    };
    lua["Factory"]["CreateNormalProjectile"] = createNormalProj;

    // CreateChargedProjectile
    std::function<ECS::Entity(float, float, int, bool, int)> createChargedProj = 
        [ctx, CreateProjectileFromWeaponConfig](float x, float y, int chargeLevel, bool isPlayer, int ownerId) -> ECS::Entity {
        std::string weaponName = isPlayer ? "single_shot" : "enemy_bullet";
        return CreateProjectileFromWeaponConfig(weaponName, x, y, isPlayer, ownerId, chargeLevel, true);
    };
    lua["Factory"]["CreateChargedProjectile"] = createChargedProj;

    // CreateExplosiveProjectile
    std::function<ECS::Entity(float, float, bool, int)> createExplosiveProj = 
        [ctx, CreateProjectileFromWeaponConfig](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        std::string weaponName = "single_shot";
        return CreateProjectileFromWeaponConfig(weaponName, x, y, isPlayer, ownerId, 1, false);
    };
    lua["Factory"]["CreateExplosiveProjectile"] = createExplosiveProj;

    // CreatePiercingProjectile
    std::function<ECS::Entity(float, float, int, bool, int)> createPiercingProj = 
        [ctx, CreateProjectileFromWeaponConfig](float x, float y, int maxPierce, bool isPlayer, int ownerId) -> ECS::Entity {
        (void)maxPierce; // parameter intentionally unused in this wrapper
        std::string weaponName = isPlayer ? "laser" : "enemy_laser";
        return CreateProjectileFromWeaponConfig(weaponName, x, y, isPlayer, ownerId, 1, false);
    };
    lua["Factory"]["CreatePiercingProjectile"] = createPiercingProj;

    // CreateHomingProjectile
    std::function<ECS::Entity(float, float, bool, int)> createHomingProj = 
        [ctx, CreateProjectileFromWeaponConfig](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        std::string weaponName = isPlayer ? "homing_missile" : "boss_homing";
        return CreateProjectileFromWeaponConfig(weaponName, x, y, isPlayer, ownerId, 1, false);
    };
    lua["Factory"]["CreateHomingProjectile"] = createHomingProj;

    // CreateLaserProjectile
    std::function<ECS::Entity(float, float, bool, int)> createLaserProj = 
        [ctx, CreateProjectileFromWeaponConfig](float x, float y, bool isPlayer, int ownerId) -> ECS::Entity {
        std::string weaponName = isPlayer ? "laser" : "enemy_laser";
        return CreateProjectileFromWeaponConfig(weaponName, x, y, isPlayer, ownerId, 1, false);
    };
    lua["Factory"]["CreateLaserProjectile"] = createLaserProj;

    // Create projectile from a weapon config name (reads WeaponsConfig projectile visual table)
    std::function<ECS::Entity(std::string, float, float, bool, int, int)> createProjFromWeapon =
        [ctx](std::string weaponName, float x, float y, bool isPlayer, int ownerId, int level) -> ECS::Entity {
        sol::state_view lua(*ctx->lua);
        sol::table weapons = lua["WeaponsConfig"];
        if (!weapons.valid()) {
            std::cerr << "[Factory] WeaponsConfig not found!" << std::endl;
            return 0;
        }
        sol::table w = weapons[weaponName];
        if (!w.valid()) {
            std::cerr << "[Factory] Weapon not found: " << weaponName << std::endl;
            return 0;
        }
        sol::table proj = w["projectile"];
        if (!proj.valid()) {
            std::cerr << "[Factory] Weapon '" << weaponName << "' has no projectile spec" << std::endl;
            return 0;
        }

        // Determine texture
        std::string texPath;
        eng::engine::rendering::sfml::SFMLTexture* tex = nullptr;
        // Safely read optional string from Lua (avoid ambiguous sol::table::get_or overload)
        try {
            sol::object texObj = proj["texture"];
            if (texObj.valid() && texObj.is<std::string>()) {
                texPath = texObj.as<std::string>();
            } else {
                texPath = std::string();
            }
        } catch (...) {
            texPath = std::string();
        }
        if (!texPath.empty()) {
            auto it = ctx->textures.find(texPath);
            if (it == ctx->textures.end() || it->second == nullptr) {
                // try to load dynamically
                auto* newTex = new eng::engine::rendering::sfml::SFMLTexture();
                std::string assetBase = lua["ASSET_BASE_PATH"].get_or(std::string(""));
                std::vector<std::string> candidates;
                if (!assetBase.empty()) {
                    if (assetBase.back() != '/' && assetBase.back() != '\\') assetBase += '/';
                    candidates.push_back(assetBase + "game/assets/" + texPath);
                    candidates.push_back(assetBase + texPath);
                }
                candidates.push_back(std::string("game/assets/") + texPath);
                candidates.push_back(texPath);
                bool loaded = false;
                for (auto& p : candidates) {
                    if (newTex->loadFromFile(p)) { loaded = true; break; }
                }
                if (!loaded) { delete newTex; }
                else { ctx->textures[texPath] = newTex; }
            }
            auto it2 = ctx->textures.find(texPath);
            if (it2 != ctx->textures.end()) tex = it2->second;
        }
        if (!tex) tex = ctx->textures["missile"]; // fallback
        if (!tex) {
            std::cerr << "[Factory] No texture available for projectile" << std::endl;
            return 0;
        }

        // Build visual spec from projectile config (normalRect / animated fields)
        ProjectileFactory::ProjectileVisualSpec spec;
        sol::table normalRect = proj["normalRect"];
        if (normalRect.valid()) {
            spec.x = normalRect.get_or("x", 0);
            spec.y = normalRect.get_or("y", 0);
            spec.w = normalRect.get_or("w", 16);
            spec.h = normalRect.get_or("h", 16);
        } else {
            spec.x = proj.get_or("x", 0);
            spec.y = proj.get_or("y", 0);
            spec.w = proj.get_or("w", 16);
            spec.h = proj.get_or("h", 16);
        }
        spec.scale = proj.get_or("scale", 1.0f);
        spec.animated = proj.get_or("animated", false);
        spec.frameCount = proj.get_or("frameCount", 1);
        spec.frameTime = proj.get_or("frameTime", 0.1f);
        spec.spacing = proj.get_or("spacing", 0);

        ECS::Entity e = ProjectileFactory::CreateProjectileFromSpec(
            *ctx->coordinator, x, y, tex, spec, *ctx->spriteList, isPlayer, ownerId, level
        );
        if (e != 0 && ctx->registerEntity) ctx->registerEntity(e);
        return e;
    };
    lua["Factory"]["CreateProjectileFromWeapon"] = createProjFromWeapon;


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
