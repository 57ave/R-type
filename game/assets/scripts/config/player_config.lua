-- ==========================================
-- R-Type Game - Player Configuration
-- ==========================================

Player = {
    -- Stats de base
    max_health = 100,
    starting_health = 100,
    speed = 600.0, -- pixels par seconde
    
    -- Dimensions et collision
    sprite = {
        width = 64,
        height = 64,
        scale = 1.0
    },
    
    collider = {
        width = 48,
        height = 40,
        offset_x = 8,
        offset_y = 12
    },
    
    -- Position de spawn
    spawn = {
        x = 200,
        y = 540, -- Centre vertical de 1080
        respawn_delay = 2.0 -- secondes
    },
    
    -- Limites de mouvement
    boundary = {
        min_x = 0,
        max_x = 1920,
        min_y = 0,
        max_y = 1080,
        margin = 20 -- pixels depuis le bord
    },
    
    -- Arme par défaut
    default_weapon = "basic_shot",
    
    -- Animation
    animation = {
        idle = {
            frames = {0, 1, 2, 3},
            frame_duration = 0.1
        },
        move_up = {
            frames = {4, 5},
            frame_duration = 0.1
        },
        move_down = {
            frames = {6, 7},
            frame_duration = 0.1
        },
        death = {
            frames = {8, 9, 10, 11},
            frame_duration = 0.15,
            loop = false
        }
    },
    
    -- Powerups
    powerups = {
        speed_boost = {
            duration = 10.0,
            multiplier = 1.5
        },
        invincibility = {
            duration = 8.0
        },
        rapid_fire = {
            duration = 12.0,
            fire_rate_multiplier = 2.0
        }
    }
}

print("[LUA] ✅ Player config loaded")

return Player
