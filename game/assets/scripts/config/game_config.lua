-- ==========================================
-- R-Type Game - Configuration Générale
-- ==========================================

Game = {
    name = "R-Type Remake",
    version = "1.0.0",
    
    window = {
        width = 1920,
        height = 1080,
        title = "R-Type - Epitech Project",
        fullscreen = false,
        vsync = true,
        fps_limit = 60
    },
    
    gameplay = {
        lives = 3,
        starting_score = 0,
        extra_life_score = 100000,
        invincibility_duration = 3.0, -- secondes après respawn
        
        -- Difficulté
        difficulty = {
            easy = {
                enemy_health_multiplier = 0.7,
                enemy_damage_multiplier = 0.7,
                score_multiplier = 0.8
            },
            normal = {
                enemy_health_multiplier = 1.0,
                enemy_damage_multiplier = 1.0,
                score_multiplier = 1.0
            },
            hard = {
                enemy_health_multiplier = 1.5,
                enemy_damage_multiplier = 1.5,
                score_multiplier = 1.5
            }
        },
        current_difficulty = "normal"
    },
    
    network = {
        server_ip = "127.0.0.1",
        server_port = 4242,
        timeout_ms = 5000,
        max_players = 4
    },
    
    audio = {
        master_volume = 100,
        music_volume = 70,
        sfx_volume = 80,
        muted = false
    },
    
    controls = {
        keyboard = {
            up = "Up",
            down = "Down",
            left = "Left",
            right = "Right",
            shoot = "Space",
            charge = "LShift",
            pause = "Escape"
        }
    },
    
    debug = {
        show_fps = true,
        show_colliders = false,
        show_network_stats = false,
        god_mode = false
    }
}

print("[LUA] ✅ Game config loaded")

return Game
