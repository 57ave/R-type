-- ==========================================
-- R-Type Game - Level 2: Rising Threat
-- Basic enemies (bug) + Bat enemies (fighter) + SecondBoss
-- Modules: spread, wave (NO homing)
-- ==========================================

Level2 = {
    id = 2,
    name = "Rising Threat",
    
    -- Enemy types allowed in this level
    -- 0=bug (basic), 1=fighter (bat), 2=kamikaze
    enemy_types = {0, 1},  -- Bugs + Bat enemies
    
    -- Module types allowed (1=homing, 3=spread, 4=wave)
    module_types = {3, 4},  -- spread and wave only
    
    -- Spawn configuration
    spawn = {
        enemy_interval = 2.0,       -- faster spawning
        powerup_interval = 12.0,
        module_interval = 22.0,
        max_enemies = 12,
    },
    
    -- Waves definition
    waves = {
        -- Wave 1: Mix of bugs and bats
        {
            time = 3.0,
            enemies = {
                {type = 0, count = 3, interval = 1.2},
                {type = 1, count = 2, interval = 1.5},
            }
        },
        -- Wave 2: More variety
        {
            time = 18.0,
            enemies = {
                {type = 0, count = 4, interval = 0.8},
                {type = 1, count = 3, interval = 1.0},
            }
        },
        -- Wave 3: Heavy bat wave
        {
            time = 35.0,
            enemies = {
                {type = 1, count = 5, interval = 0.7},
                {type = 0, count = 3, interval = 1.0},
            }
        },
        -- Wave 4: Big swarm
        {
            time = 55.0,
            enemies = {
                {type = 0, count = 6, interval = 0.5},
                {type = 1, count = 4, interval = 0.6},
            }
        },
        -- Wave 5: Pre-boss pressure
        {
            time = 75.0,
            enemies = {
                {type = 0, count = 8, interval = 0.4},
                {type = 1, count = 5, interval = 0.5},
            }
        },
    },
    
    -- Boss configuration
    boss = {
        spawn_time = 95.0,
        type = 4,                   -- enemyType 4 = SecondBoss
        name = "SecondBoss",
        health = 400,
        speed = 60.0,
        fire_rate = 1.5,
        fire_pattern = 2,          -- circle
        sprite = {
            path = "assets/enemies/SecondBoss.png",
            frame_width = 215,
            frame_height = 211,
            frame_count = 3,
            frame_time = 0.12,
            scale = 1.5,
            vertical = false,       -- Horizontal spritesheet
        },
    },
    
    stop_spawning_at_boss = true,
}

print("[LUA] Level 2 loaded")
return Level2
