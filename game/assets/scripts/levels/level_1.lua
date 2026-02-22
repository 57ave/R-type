-- ==========================================
-- R-Type Game - Level 1: First Contact
-- Only basic enemies (bug) + FirstBoss
-- Modules: spread, wave (NO homing)
-- ==========================================

Level1 = {
    id = 1,
    name = "First Contact",

    -- Enemy types allowed in this level
    -- 0=bug (basic), 1=fighter (bat), 2=kamikaze
    enemy_types = {0},  -- Only bugs

    -- Module types allowed (1=homing, 3=spread, 4=wave)
    module_types = {3, 4},  -- spread and wave only

    -- Spawn configuration
    spawn = {
        enemy_interval = 2.5,       -- seconds between enemy spawns
        powerup_interval = 15.0,    -- seconds between powerup spawns
        module_interval = 25.0,     -- seconds between module spawns
        max_enemies = 8,            -- max enemies on screen at once
    },

    -- Waves definition
    -- Each wave has: time (seconds since level start), enemies list
    waves = {
        -- Wave 1: Introduction - few basic enemies
        {
            time = 3.0,
            enemies = {
                {type = 0, count = 3, interval = 1.5},  -- 3 bugs
            }
        },
        -- Wave 2: More bugs
        {
            time = 15.0,
            enemies = {
                {type = 0, count = 5, interval = 1.0},
            }
        },
        -- Wave 3: Swarm
        {
            time = 30.0,
            enemies = {
                {type = 0, count = 6, interval = 0.8},
            }
        },
        -- Wave 4: Bigger swarm
        {
            time = 50.0,
            enemies = {
                {type = 0, count = 8, interval = 0.6},
            }
        },
        -- Wave 5: Pre-boss swarm
        {
            time = 70.0,
            enemies = {
                {type = 0, count = 10, interval = 0.5},
            }
        },
    },

    -- Boss configuration
    boss = {
        spawn_time = 90.0,          -- Boss appears at 90 seconds
        type = 3,                   -- enemyType 3 = FirstBoss
        name = "FirstBoss",
        health = 200,
        speed = 80.0,
        fire_rate = 2.0,
        fire_pattern = 0,           -- straight
        sprite = {
            path = "assets/enemies/FirstBoss.png",
            frame_width = 259,
            frame_height = 191,
            frame_count = 3,
            frame_time = 0.15,
            scale = 1.5,
            vertical = true,        -- Vertical spritesheet
        },
    },

    -- Stop regular spawning when boss arrives
    stop_spawning_at_boss = true,
}

print("[LUA]  Level 1 loaded")
return Level1
