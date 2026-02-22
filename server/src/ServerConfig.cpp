#include "ServerConfig.hpp"
#include "core/Logger.hpp"
#include <filesystem>

#if SERVER_SCRIPTING_ENABLED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wtemplate-body"
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#pragma GCC diagnostic pop
#endif

namespace ServerConfig {

bool loadFromLua(Config& config, const std::string& luaPath) {
#if SERVER_SCRIPTING_ENABLED
    // Check file exists
    if (!std::filesystem::exists(luaPath)) {
        LOG_WARNING("SERVERCONFIG", " Config file not found: " + luaPath);
        LOG_WARNING("SERVERCONFIG", "Using default values.");
        return false;
    }

    try {
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::io);

        auto result = lua.safe_script_file(luaPath, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            LOG_ERROR("SERVERCONFIG", "Failed to load Lua config: " + std::string(err.what()));
            return false;
        }

        sol::table cfg = lua["ServerConfig"];
        if (!cfg.valid()) {
            LOG_WARNING("SERVERCONFIG", "'ServerConfig' table not found in " + luaPath);
            return false;
        }

        // ---- Player ----
        sol::optional<sol::table> playerT = cfg["player"];
        if (playerT) {
            auto& p = config.player;
            p.speed      = playerT.value().get_or("speed", p.speed);
            p.maxHealth   = playerT.value().get_or("max_health", p.maxHealth);
            p.spawnX      = playerT.value().get_or("spawn_x", p.spawnX);
            p.spawnYStart = playerT.value().get_or("spawn_y_start", p.spawnYStart);
            p.spawnYOffset = playerT.value().get_or("spawn_y_offset", p.spawnYOffset);

            sol::optional<sol::table> bd = playerT.value()["boundary"];
            if (bd) {
                p.boundaryMinX = bd.value().get_or("min_x", p.boundaryMinX);
                p.boundaryMinY = bd.value().get_or("min_y", p.boundaryMinY);
                p.boundaryMaxX = bd.value().get_or("max_x", p.boundaryMaxX);
                p.boundaryMaxY = bd.value().get_or("max_y", p.boundaryMaxY);
            }
        }

        // ---- Helper to load enemy type config ----
        auto loadEnemyType = [](sol::table& tbl, EnemyTypeConfig& e) {
            e.typeId          = tbl.get_or("type_id", (int)e.typeId);
            e.health          = tbl.get_or("health", e.health);
            e.vx              = tbl.get_or("vx", e.vx);
            e.vy              = tbl.get_or("vy", e.vy);
            e.firePattern     = tbl.get_or("fire_pattern", (int)e.firePattern);
            e.fireRate        = tbl.get_or("fire_rate", e.fireRate);
            e.collisionDamage = tbl.get_or("collision_damage", e.collisionDamage);
            e.score           = tbl.get_or("score", e.score);
            e.zigzagInterval  = tbl.get_or("zigzag_interval", e.zigzagInterval);
            e.boundaryTop     = tbl.get_or("boundary_top", e.boundaryTop);
            e.boundaryBottom  = tbl.get_or("boundary_bottom", e.boundaryBottom);
            e.trackingSpeed   = tbl.get_or("tracking_speed", e.trackingSpeed);
        };

        // ---- Enemies ----
        sol::optional<sol::table> enemiesT = cfg["enemies"];
        if (enemiesT) {
            sol::optional<sol::table> bugT = enemiesT.value()["bug"];
            if (bugT) { auto t = bugT.value(); loadEnemyType(t, config.bug); }

            sol::optional<sol::table> fighterT = enemiesT.value()["fighter"];
            if (fighterT) { auto t = fighterT.value(); loadEnemyType(t, config.fighter); }

            sol::optional<sol::table> kamikazeT = enemiesT.value()["kamikaze"];
            if (kamikazeT) { auto t = kamikazeT.value(); loadEnemyType(t, config.kamikaze); }

            auto& es = config.enemySpawn;
            es.spawnX            = enemiesT.value().get_or("spawn_x", es.spawnX);
            es.spawnYMin         = enemiesT.value().get_or("spawn_y_min", es.spawnYMin);
            es.spawnYRange       = enemiesT.value().get_or("spawn_y_range", es.spawnYRange);
            es.fireTimerBase     = enemiesT.value().get_or("fire_timer_base", es.fireTimerBase);
            es.fireTimerRandomRange = enemiesT.value().get_or("fire_timer_random_range", es.fireTimerRandomRange);
        }

        // ---- Bosses ----
        sol::optional<sol::table> bossesT = cfg["bosses"];
        if (bossesT) {
            auto& b = config.bossMovement;
            b.spawnX                    = bossesT.value().get_or("spawn_x", b.spawnX);
            b.spawnY                    = bossesT.value().get_or("spawn_y", b.spawnY);
            b.stopX                     = bossesT.value().get_or("stop_x", b.stopX);
            b.bobSpeed                  = bossesT.value().get_or("bob_speed", b.bobSpeed);
            b.bobAmplitude              = bossesT.value().get_or("bob_amplitude", b.bobAmplitude);
            b.boundaryTop               = bossesT.value().get_or("boundary_top", b.boundaryTop);
            b.boundaryBottom            = bossesT.value().get_or("boundary_bottom", b.boundaryBottom);
            b.score                     = bossesT.value().get_or("score", b.score);
            b.collisionDamageToPlayer   = bossesT.value().get_or("collision_damage_to_player", b.collisionDamageToPlayer);
            b.collisionDamageFromPlayer = bossesT.value().get_or("collision_damage_from_player", b.collisionDamageFromPlayer);
        }

        // ---- Projectiles ----
        sol::optional<sol::table> projT = cfg["projectiles"];
        if (projT) {
            sol::optional<sol::table> ppT = projT.value()["player"];
            if (ppT) {
                auto& pp = config.projectiles.player;
                pp.normalSpeed           = ppT.value().get_or("normal_speed", pp.normalSpeed);
                pp.chargedSpeed          = ppT.value().get_or("charged_speed", pp.chargedSpeed);
                pp.baseDamage            = ppT.value().get_or("base_damage", pp.baseDamage);
                pp.chargeDamageMultiplier = ppT.value().get_or("charge_damage_multiplier", pp.chargeDamageMultiplier);
                pp.fireCooldownNormal    = ppT.value().get_or("fire_cooldown_normal", pp.fireCooldownNormal);
                pp.fireCooldownCharged   = ppT.value().get_or("fire_cooldown_charged", pp.fireCooldownCharged);
                pp.spawnOffsetX          = ppT.value().get_or("spawn_offset_x", pp.spawnOffsetX);
                pp.spawnOffsetY          = ppT.value().get_or("spawn_offset_y", pp.spawnOffsetY);
            }
            sol::optional<sol::table> epT = projT.value()["enemy"];
            if (epT) {
                auto& ep = config.projectiles.enemy;
                ep.speedMultiplier   = epT.value().get_or("speed_multiplier", ep.speedMultiplier);
                ep.minSpeed          = epT.value().get_or("min_speed", ep.minSpeed);
                ep.circleCount       = epT.value().get_or("circle_count", ep.circleCount);
                ep.circleSpeedFactor = epT.value().get_or("circle_speed_factor", ep.circleSpeedFactor);
                ep.spreadAngle       = epT.value().get_or("spread_angle", ep.spreadAngle);
                ep.spawnOffsetX      = epT.value().get_or("spawn_offset_x", ep.spawnOffsetX);
            }
            config.projectiles.missileDamage = projT.value().get_or("missile_damage", config.projectiles.missileDamage);
        }

        // ---- Modules ----
        sol::optional<sol::table> modT = cfg["modules"];
        if (modT) {
            auto& m = config.modules;
            m.fireCooldown = modT.value().get_or("fire_cooldown", m.fireCooldown);
            m.baseSpeed    = modT.value().get_or("base_speed", m.baseSpeed);
            m.spawnVx      = modT.value().get_or("spawn_vx", m.spawnVx);

            sol::optional<sol::table> hT = modT.value()["homing"];
            if (hT) {
                m.homing.speed           = hT.value().get_or("speed", m.homing.speed);
                m.homing.detectionRadius = hT.value().get_or("detection_radius", m.homing.detectionRadius);
                m.homing.turnRate        = hT.value().get_or("turn_rate", m.homing.turnRate);
                m.homing.projectileType  = hT.value().get_or("projectile_type", (int)m.homing.projectileType);
            }
            sol::optional<sol::table> sT = modT.value()["spread"];
            if (sT) {
                m.spread.projectileType = sT.value().get_or("projectile_type", (int)m.spread.projectileType);
                sol::optional<sol::table> anglesT = sT.value()["angles"];
                if (anglesT) {
                    m.spread.angles.clear();
                    for (auto& kv : anglesT.value()) {
                        m.spread.angles.push_back(kv.second.as<float>());
                    }
                }
            }
            sol::optional<sol::table> wT = modT.value()["wave"];
            if (wT) {
                m.wave.amplitude      = wT.value().get_or("amplitude", m.wave.amplitude);
                m.wave.frequency      = wT.value().get_or("frequency", m.wave.frequency);
                m.wave.projectileType = wT.value().get_or("projectile_type", (int)m.wave.projectileType);
            }
        }

        // ---- Powerups ----
        sol::optional<sol::table> puT = cfg["powerups"];
        if (puT) {
            auto& pu = config.powerups;
            pu.spawnVx    = puT.value().get_or("spawn_vx", pu.spawnVx);
            pu.spawnX     = puT.value().get_or("spawn_x", pu.spawnX);
            pu.spawnYMin  = puT.value().get_or("spawn_y_min", pu.spawnYMin);
            pu.spawnYRange = puT.value().get_or("spawn_y_range", pu.spawnYRange);

            sol::optional<sol::table> orT = puT.value()["orange"];
            if (orT) {
                pu.orange.bossDamageFraction = orT.value().get_or("boss_damage_fraction", pu.orange.bossDamageFraction);
            }
            sol::optional<sol::table> blT = puT.value()["blue"];
            if (blT) {
                pu.blue.duration = blT.value().get_or("duration", pu.blue.duration);
            }
        }

        // ---- Explosions ----
        sol::optional<sol::table> exT = cfg["explosions"];
        if (exT) {
            config.explosions.lifetime = exT.value().get_or("lifetime", config.explosions.lifetime);
        }

        // ---- Collisions ----
        sol::optional<sol::table> colT = cfg["collisions"];
        if (colT) {
            auto& c = config.collisions;
            c.hitboxSize    = colT.value().get_or("hitbox_size", c.hitboxSize);
            c.oobMargin     = colT.value().get_or("oob_margin", c.oobMargin);
            c.screenWidth   = colT.value().get_or("screen_width", c.screenWidth);
            c.screenHeight  = colT.value().get_or("screen_height", c.screenHeight);
        }

        // ---- Levels ----
        sol::optional<sol::table> levelsT = cfg["levels"];
        if (levelsT) {
            config.maxLevel = levelsT.value().get_or("max_level", config.maxLevel);
            config.levels.clear();

            for (int lvl = 1; lvl <= config.maxLevel; ++lvl) {
                sol::optional<sol::table> lT = levelsT.value()[lvl];
                if (!lT) continue;

                LevelDefinition def;
                def.id = lvl;
                def.name = lT.value().get_or("name", std::string("Level " + std::to_string(lvl)));
                def.enemyInterval = lT.value().get_or("enemy_interval", 2.0f);
                def.powerupInterval = lT.value().get_or("powerup_interval", 15.0f);
                def.moduleInterval = lT.value().get_or("module_interval", 25.0f);
                def.maxEnemies = lT.value().get_or("max_enemies", 8);
                def.stopSpawningAtBoss = lT.value().get_or("stop_spawning_at_boss", true);

                // Enemy types
                sol::optional<sol::table> etT = lT.value()["enemy_types"];
                if (etT) {
                    for (auto& kv : etT.value()) {
                        def.enemyTypes.push_back(static_cast<uint8_t>(kv.second.as<int>()));
                    }
                }

                // Module types
                sol::optional<sol::table> mtT = lT.value()["module_types"];
                if (mtT) {
                    for (auto& kv : mtT.value()) {
                        def.moduleTypes.push_back(static_cast<uint8_t>(kv.second.as<int>()));
                    }
                }

                // Waves
                sol::optional<sol::table> wavesT = lT.value()["waves"];
                if (wavesT) {
                    for (auto& wkv : wavesT.value()) {
                        sol::table waveT = wkv.second.as<sol::table>();
                        WaveDefinition wd;
                        wd.time = waveT.get_or("time", 0.0f);

                        sol::optional<sol::table> groupsT = waveT["groups"];
                        if (groupsT) {
                            for (auto& gkv : groupsT.value()) {
                                sol::table gt = gkv.second.as<sol::table>();
                                WaveEnemyGroup g;
                                g.type     = static_cast<uint8_t>(gt.get_or("type", 0));
                                g.count    = gt.get_or("count", 1);
                                g.interval = gt.get_or("interval", 1.0f);
                                wd.groups.push_back(g);
                            }
                        }
                        def.waves.push_back(wd);
                    }
                }

                // Boss
                sol::optional<sol::table> bossT = lT.value()["boss"];
                if (bossT) {
                    def.boss.enemyType   = static_cast<uint8_t>(bossT.value().get_or("enemy_type", 3));
                    def.boss.health      = static_cast<uint16_t>(bossT.value().get_or("health", 1000));
                    def.boss.speed       = bossT.value().get_or("speed", 80.0f);
                    def.boss.fireRate    = bossT.value().get_or("fire_rate", 2.0f);
                    def.boss.firePattern = static_cast<uint8_t>(bossT.value().get_or("fire_pattern", 0));
                    def.boss.spawnTime   = bossT.value().get_or("spawn_time", 90.0f);
                }

                config.levels.push_back(def);
            }
        }

        // ---- Server settings ----
        sol::optional<sol::table> srvT = cfg["server"];
        if (srvT) {
            auto& s = config.server;
            s.serverIp          = srvT.value().get_or<std::string>("server_ip", s.serverIp);
            s.port              = srvT.value().get_or("port", s.port);
            s.tickRate          = srvT.value().get_or("tick_rate", s.tickRate);
            s.snapshotRate      = srvT.value().get_or("snapshot_rate", s.snapshotRate);
            s.minPlayersToStart = srvT.value().get_or("min_players_to_start", s.minPlayersToStart);
            s.maxPlayerShips    = srvT.value().get_or("max_player_ships", s.maxPlayerShips);
        }

        LOG_INFO("SERVERCONFIG", " Loaded config from " + luaPath);
        LOG_INFO("SERVERCONFIG", "  Player speed=" + std::to_string(config.player.speed) + " hp=" + std::to_string(config.player.maxHealth));
        LOG_INFO("SERVERCONFIG", "  Levels: " + std::to_string(config.levels.size()));
        for (size_t i = 0; i < config.levels.size(); ++i) {
            const auto& l = config.levels[i];
            LOG_INFO("SERVERCONFIG", "    L" + std::to_string(l.id) + ": " + l.name + " (boss HP=" + std::to_string(l.boss.health) + ", waves=" + std::to_string(l.waves.size()) + ")");
        }
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("SERVERCONFIG", "Exception loading Lua config: " + std::string(e.what()));
        return false;
    }
#else
    (void)luaPath;
    LOG_INFO("SERVERCONFIG", "Lua scripting not enabled, using defaults");
    return false;
#endif
}

} // namespace ServerConfig
