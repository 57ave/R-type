-- ==========================================
-- R-Type Game - Level 3: Final Assault
-- All enemies: bug + bat + kamikaze + LastBoss
-- ALL Modules: spread, wave, AND homing (exclusive to level 3!)
-- ==========================================
-- ⚠️ THIS IS THE SINGLE SOURCE OF TRUTH for Level 3.
-- Used by BOTH solo (PlayState) and multiplayer (server).

Level3 = {
    name = "Final Assault",

    -- Enemy types allowed: 0=bug, 1=fighter(bat), 2=kamikaze
    enemy_types = {0, 1, 2},

    -- Module types allowed: 1=homing, 3=spread, 4=wave
    module_types = {1, 3, 4},

    -- Spawn configuration
    enemy_interval = 1.5,
    powerup_interval = 10.0,
    module_interval = 20.0,
    max_enemies = 15,
    stop_spawning_at_boss = true,

    -- Waves definition (groups format for server compatibility)
    waves = {
        {time = 3.0,  groups = {{type = 0, count = 4, interval = 0.8}, {type = 1, count = 3, interval = 1.0}, {type = 2, count = 2, interval = 1.2}}},
        {time = 18.0, groups = {{type = 2, count = 5, interval = 0.6}, {type = 0, count = 3, interval = 0.8}}},
        {time = 35.0, groups = {{type = 0, count = 5, interval = 0.5}, {type = 1, count = 4, interval = 0.6}, {type = 2, count = 3, interval = 0.7}}},
        {time = 55.0, groups = {{type = 0, count = 8, interval = 0.3}, {type = 1, count = 5, interval = 0.4}, {type = 2, count = 4, interval = 0.5}}},
        {time = 75.0, groups = {{type = 0, count = 10, interval = 0.3}, {type = 1, count = 6, interval = 0.4}, {type = 2, count = 5, interval = 0.4}}},
    },

    -- Boss configuration
    boss = {
        enemy_type = 5,
        health = 3000,
        speed = 100.0,
        fire_rate = 1.0,
        fire_pattern = 3,
        spawn_time = 95.0,
        -- Client-side rendering info (ignored by server)
        name = "LastBoss",
        sprite = {
            path = "assets/enemies/LastBossFly.png",
            frame_width = 81,
            frame_height = 71,
            frame_count = 4,
            frame_time = 0.1,
            scale = 2.5,
            vertical = false,
        },
    },
}

print("[LUA] Level 3 loaded")
return Level3
