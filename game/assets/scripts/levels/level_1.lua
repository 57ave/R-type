-- ==========================================
-- R-Type Game - Level 1
-- ==========================================

Level1 = {
    id = "level_1",
    name = "Asteroid Field",
    
    -- Configuration générale
    config = {
        duration = 300, -- secondes (5 minutes)
        background = {
            layers = {
                {texture = "layer_1", scroll_speed = 20},
                {texture = "layer_2", scroll_speed = 40},
                {texture = "layer_3", scroll_speed = 60},
                {texture = "layer_4", scroll_speed = 100}
            }
        },
        music = "level_1",
        difficulty_multiplier = 1.0
    },
    
    -- Waves d'ennemis
    waves = {
        {
            time = 5.0,
            enemies = {
                {type = "basic_enemy", count = 3, formation = "line", spawn_interval = 1.0}
            }
        },
        {
            time = 15.0,
            enemies = {
                {type = "basic_enemy", count = 5, formation = "V", spawn_interval = 0.5}
            }
        },
        {
            time = 30.0,
            enemies = {
                {type = "fast_enemy", count = 4, formation = "diamond", spawn_interval = 0.8},
                {type = "basic_enemy", count = 2, formation = "sides", spawn_interval = 2.0}
            }
        },
        {
            time = 50.0,
            enemies = {
                {type = "tank_enemy", count = 1, formation = "single", spawn_interval = 0},
                {type = "basic_enemy", count = 6, formation = "swarm", spawn_interval = 0.3}
            }
        },
        {
            time = 70.0,
            enemies = {
                {type = "kamikaze_enemy", count = 5, formation = "wave", spawn_interval = 0.5}
            }
        },
        {
            time = 90.0,
            enemies = {
                {type = "sniper_enemy", count = 2, formation = "corners", spawn_interval = 2.0},
                {type = "fast_enemy", count = 4, formation = "circle", spawn_interval = 0.5}
            }
        },
        {
            time = 120.0,
            enemies = {
                {type = "tank_enemy", count = 2, formation = "pincer", spawn_interval = 3.0},
                {type = "basic_enemy", count = 8, formation = "grid", spawn_interval = 0.4}
            }
        },
        {
            time = 150.0,
            enemies = {
                {type = "basic_enemy", count = 10, formation = "chaos", spawn_interval = 0.2},
                {type = "fast_enemy", count = 5, formation = "chase", spawn_interval = 0.6},
                {type = "kamikaze_enemy", count = 3, formation = "line", spawn_interval = 1.0}
            }
        },
        {
            time = 180.0,
            boss = "guardian_boss"
        }
    },
    
    -- Événements spéciaux
    events = {
        {
            time = 60.0,
            type = "powerup_spawn",
            powerup = "weapon_upgrade",
            position = {x = 1920, y = 540}
        },
        {
            time = 100.0,
            type = "message",
            text = "Warning: Heavy enemies detected!",
            duration = 3.0
        },
        {
            time = 175.0,
            type = "message",
            text = "BOSS APPROACHING!",
            duration = 4.0,
            warning = true
        }
    }
}

print("[LUA] ✅ Level 1 loaded")

return Level1
