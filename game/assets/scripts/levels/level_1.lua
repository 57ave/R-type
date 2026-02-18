-- ==========================================-- ==========================================

-- R-Type Game - Level 1: First Contact-- R-Type Game - Level 1

-- Only basic enemies (bug) + FirstBoss-- ==========================================

-- Modules: spread, wave (NO homing)

-- ==========================================Level1 = {

    id = "level_1",

Level1 = {    name = "Asteroid Field",

    id = 1,    

    name = "First Contact",    -- Configuration générale

        config = {

    -- Enemy types allowed in this level        duration = 300, -- secondes (5 minutes)

    -- 0=bug (basic), 1=fighter (bat), 2=kamikaze        background = {

    enemy_types = {0},  -- Only bugs            layers = {

                    {texture = "layer_1", scroll_speed = 20},

    -- Module types allowed (1=homing, 3=spread, 4=wave)                {texture = "layer_2", scroll_speed = 40},

    module_types = {3, 4},  -- spread and wave only                {texture = "layer_3", scroll_speed = 60},

                    {texture = "layer_4", scroll_speed = 100}

    -- Spawn configuration            }

    spawn = {        },

        enemy_interval = 2.5,       -- seconds between enemy spawns (easier)        music = "level_1",

        powerup_interval = 15.0,    -- seconds between powerup spawns        difficulty_multiplier = 1.0

        module_interval = 25.0,     -- seconds between module spawns    },

        max_enemies = 8,            -- max enemies on screen at once    

    },    -- Waves d'ennemis

        waves = {

    -- Waves definition        {

    -- Each wave has: time (seconds since level start), enemies list            time = 5.0,

    waves = {            enemies = {

        -- Wave 1: Introduction - few basic enemies                {type = "basic_enemy", count = 3, formation = "line", spawn_interval = 1.0}

        {            }

            time = 3.0,        },

            enemies = {        {

                {type = 0, count = 3, interval = 1.5},  -- 3 bugs            time = 15.0,

            }            enemies = {

        },                {type = "basic_enemy", count = 5, formation = "V", spawn_interval = 0.5}

        -- Wave 2: More bugs            }

        {        },

            time = 15.0,        {

            enemies = {            time = 30.0,

                {type = 0, count = 5, interval = 1.0},            enemies = {

            }                {type = "fast_enemy", count = 4, formation = "diamond", spawn_interval = 0.8},

        },                {type = "basic_enemy", count = 2, formation = "sides", spawn_interval = 2.0}

        -- Wave 3: Swarm            }

        {        },

            time = 30.0,        {

            enemies = {            time = 50.0,

                {type = 0, count = 6, interval = 0.8},            enemies = {

            }                {type = "tank_enemy", count = 1, formation = "single", spawn_interval = 0},

        },                {type = "basic_enemy", count = 6, formation = "swarm", spawn_interval = 0.3}

        -- Wave 4: Bigger swarm            }

        {        },

            time = 50.0,        {

            enemies = {            time = 70.0,

                {type = 0, count = 8, interval = 0.6},            enemies = {

            }                {type = "kamikaze_enemy", count = 5, formation = "wave", spawn_interval = 0.5}

        },            }

        -- Wave 5: Pre-boss swarm        },

        {        {

            time = 70.0,            time = 90.0,

            enemies = {            enemies = {

                {type = 0, count = 10, interval = 0.5},                {type = "sniper_enemy", count = 2, formation = "corners", spawn_interval = 2.0},

            }                {type = "fast_enemy", count = 4, formation = "circle", spawn_interval = 0.5}

        },            }

    },        },

            {

    -- Boss configuration            time = 120.0,

    boss = {            enemies = {

        spawn_time = 90.0,         -- Boss appears at 90 seconds                {type = "tank_enemy", count = 2, formation = "pincer", spawn_interval = 3.0},

        type = 3,                   -- enemyType 3 = FirstBoss                {type = "basic_enemy", count = 8, formation = "grid", spawn_interval = 0.4}

        name = "FirstBoss",            }

        health = 200,        },

        speed = 80.0,        {

        fire_rate = 2.0,            time = 150.0,

        fire_pattern = 0,          -- straight            enemies = {

        sprite = {                {type = "basic_enemy", count = 10, formation = "chaos", spawn_interval = 0.2},

            path = "assets/enemies/FirstBoss.png",                {type = "fast_enemy", count = 5, formation = "chase", spawn_interval = 0.6},

            frame_width = 259,                {type = "kamikaze_enemy", count = 3, formation = "line", spawn_interval = 1.0}

            frame_height = 191,            }

            frame_count = 3,        },

            frame_time = 0.15,        {

            scale = 1.5,            time = 180.0,

            vertical = true,        -- Vertical spritesheet            boss = "guardian_boss"

        },        }

    },    },

        

    -- Stop regular spawning when boss arrives    -- Événements spéciaux

    stop_spawning_at_boss = true,    events = {

}        {

            time = 60.0,

print("[LUA] ✅ Level 1 loaded")            type = "powerup_spawn",

return Level1            powerup = "weapon_upgrade",

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
