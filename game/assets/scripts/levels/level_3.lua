-- ==========================================
-- R-Type Game - Level 3: Final Assault
-- All enemies: bug + bat + kamikaze + LastBoss
-- ALL Modules: spread, wave, AND homing (exclusive to level 3!)
-- ==========================================

Level3 = {
    id = 3,
    name = "Final Assault",
    
    -- Enemy types allowed in this level
    -- 0=bug (basic), 1=fighter (bat), 2=kamikaze
    enemy_types = {0, 1, 2},  -- ALL enemy types!
    
    -- Module types allowed (1=homing, 3=spread, 4=wave)
    module_types = {1, 3, 4},  -- ALL modules including homing!
    
    -- Spawn configuration
    spawn = {
        enemy_interval = 1.5,       -- aggressive spawning
        powerup_interval = 10.0,
        module_interval = 20.0,
        max_enemies = 15,
    },
    
    -- Waves definition
    waves = {
        -- Wave 1: Mixed wave
        {
            time = 3.0,
            enemies = {
                {type = 0, count = 4, interval = 0.8},
                {type = 1, count = 3, interval = 1.0},
                {type = 2, count = 2, interval = 1.2},
            }
        },
        -- Wave 2: Kamikaze assault
        {
            time = 18.0,
            enemies = {
                {type = 2, count = 5, interval = 0.6},
                {type = 0, count = 3, interval = 0.8},
            }
        },
        -- Wave 3: Full chaos
        {
            time = 35.0,
            enemies = {
                {type = 0, count = 5, interval = 0.5},
                {type = 1, count = 4, interval = 0.6},
                {type = 2, count = 3, interval = 0.7},
            }
        },
        -- Wave 4: Massive swarm
        {
            time = 55.0,
            enemies = {
                {type = 0, count = 8, interval = 0.3},
                {type = 1, count = 5, interval = 0.4},
                {type = 2, count = 4, interval = 0.5},
            }
        },
        -- Wave 5: Pre-boss hell
        {
            time = 75.0,
            enemies = {
                {type = 0, count = 10, interval = 0.3},
                {type = 1, count = 6, interval = 0.4},
                {type = 2, count = 5, interval = 0.4},
            }
        },
    },
    
    -- Boss configuration
    boss = {
        spawn_time = 95.0,
        type = 5,                   -- enemyType 5 = LastBoss
        name = "LastBoss",
        health = 600,
        speed = 100.0,
        fire_rate = 1.0,
        fire_pattern = 3,          -- spread
        sprite = {
            path = "assets/enemies/LastBossFly.png",
            frame_width = 81,
            frame_height = 71,
            frame_count = 4,
            frame_time = 0.1,
            scale = 2.5,
            vertical = false,       -- Horizontal spritesheet
        },
    },
    
    stop_spawning_at_boss = true,
}

print("[LUA] Level 3 loaded")
return Level3
