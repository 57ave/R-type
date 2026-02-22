-- ==========================================
-- R-Type Game - Level 1: First Contact
-- Only basic enemies (bug) + FirstBoss
-- Modules: spread, wave (NO homing)
-- ==========================================
-- ⚠️ THIS IS THE SINGLE SOURCE OF TRUTH for Level 1.
-- Used by BOTH solo (PlayState) and multiplayer (server).

Level1 = {
    name = "First Contact",

    -- Enemy types allowed: 0=bug, 1=fighter(bat), 2=kamikaze
    enemy_types = {0},

    -- Module types allowed: 1=homing, 3=spread, 4=wave
    module_types = {3, 4},

    -- Spawn configuration
    enemy_interval = 2.5,
    powerup_interval = 15.0,
    module_interval = 25.0,
    max_enemies = 8,
    stop_spawning_at_boss = true,

    -- Waves definition (groups format for server compatibility)
    waves = {
        {time = 3.0,  groups = {{type = 0, count = 3, interval = 1.5}}},
        {time = 15.0, groups = {{type = 0, count = 5, interval = 1.0}}},
        {time = 30.0, groups = {{type = 0, count = 6, interval = 0.8}}},
        {time = 50.0, groups = {{type = 0, count = 8, interval = 0.6}}},
        {time = 70.0, groups = {{type = 0, count = 10, interval = 0.5}}},
    },

    -- Boss configuration
    boss = {
        enemy_type = 3,
        health = 1000,
        speed = 80.0,
        fire_rate = 2.0,
        fire_pattern = 0,
        spawn_time = 90.0,
        -- Client-side rendering info (ignored by server)
        name = "FirstBoss",
        sprite = {
            path = "assets/enemies/FirstBoss.png",
            frame_width = 259,
            frame_height = 191,
            frame_count = 3,
            frame_time = 0.15,
            scale = 1.5,
            vertical = true,
        },
    },
}

print("[LUA] Level 1 loaded")
return Level1
