-- ==========================================
-- R-Type Game - Level 2: Rising Threat
-- Basic enemies (bug) + Bat enemies (fighter) + SecondBoss
-- Modules: spread, wave (NO homing)
-- ==========================================
-- ⚠️ THIS IS THE SINGLE SOURCE OF TRUTH for Level 2.
-- Used by BOTH solo (PlayState) and multiplayer (server).

Level2 = {
    name = "Rising Threat",

    -- Enemy types allowed: 0=bug, 1=fighter(bat), 2=kamikaze
    enemy_types = {0, 1},

    -- Module types allowed: 1=homing, 3=spread, 4=wave
    module_types = {3, 4},

    -- Spawn configuration
    enemy_interval = 2.0,
    powerup_interval = 12.0,
    module_interval = 22.0,
    max_enemies = 12,
    stop_spawning_at_boss = true,

    -- Waves definition (groups format for server compatibility)
    waves = {
        {time = 3.0,  groups = {{type = 0, count = 3, interval = 1.2}, {type = 1, count = 2, interval = 1.5}}},
        {time = 18.0, groups = {{type = 0, count = 4, interval = 0.8}, {type = 1, count = 3, interval = 1.0}}},
        {time = 35.0, groups = {{type = 1, count = 5, interval = 0.7}, {type = 0, count = 3, interval = 1.0}}},
        {time = 55.0, groups = {{type = 0, count = 6, interval = 0.5}, {type = 1, count = 4, interval = 0.6}}},
        {time = 75.0, groups = {{type = 0, count = 8, interval = 0.4}, {type = 1, count = 5, interval = 0.5}}},
    },

    -- Boss configuration
    boss = {
        enemy_type = 4,
        health = 2000,
        speed = 60.0,
        fire_rate = 1.5,
        fire_pattern = 2,
        spawn_time = 95.0,
        -- Client-side rendering info (ignored by server)
        name = "SecondBoss",
        sprite = {
            path = "assets/enemies/SecondBoss.png",
            frame_width = 215,
            frame_height = 211,
            frame_count = 3,
            frame_time = 0.12,
            scale = 1.5,
            vertical = false,
        },
    },
}

print("[LUA] Level 2 loaded")
return Level2
