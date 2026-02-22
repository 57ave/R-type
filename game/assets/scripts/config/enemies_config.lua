-- ==========================================
-- R-Type Game - Enemies Configuration
-- ==========================================

Enemies = {
    -- Ennemi de base (grunt)
    basic_enemy = {
        name = "Basic Enemy",
        health = 20,
        damage = 10,
        speed = 150.0,
        score_value = 100,
        
        sprite = {
            width = 48,
            height = 48,
            texture = "enemy_basic",
            scale = 1.0
        },
        
        collider = {
            width = 44,
            height = 44
        },
        
        animation = {
            fly = {
                frames = {0, 1, 2, 3},
                frame_duration = 0.1
            },
            death = {
                frames = {4, 5, 6, 7, 8},
                frame_duration = 0.1,
                loop = false
            }
        },
        
        -- Pattern de mouvement
        movement_pattern = "straight_left",
        
        -- Comportement de tir
        weapon = {
            type = "basic_shot",
            fire_rate = 2.0,
            accuracy = 0.8
        },
        
        -- Sons
        sounds = {
            spawn = "enemy_spawn",
            death = "enemy_death",
            shoot = "enemy_shoot"
        },
        
        -- Drops possibles
        drops = {
            {item = "score_bonus", chance = 0.3},
            {item = "health_pack", chance = 0.1}
        }
    },
    
    -- Ennemi rapide
    fast_enemy = {
        name = "Fast Enemy",
        health = 10,
        damage = 15,
        speed = 300.0,
        score_value = 150,
        
        sprite = {
            width = 40,
            height = 40,
            texture = "enemy_fast",
            scale = 1.0
        },
        
        collider = {
            width = 36,
            height = 36
        },
        
        animation = {
            fly = {
                frames = {0, 1, 2},
                frame_duration = 0.08
            },
            death = {
                frames = {3, 4, 5, 6},
                frame_duration = 0.1,
                loop = false
            }
        },
        
        movement_pattern = "zigzag",
        
        weapon = {
            type = "basic_shot",
            fire_rate = 1.5,
            accuracy = 0.6
        },
        
        sounds = {
            spawn = "enemy_spawn",
            death = "enemy_death_fast",
            shoot = "enemy_shoot"
        },
        
        drops = {
            {item = "score_bonus", chance = 0.4},
            {item = "speed_boost", chance = 0.05}
        }
    },
    
    -- Ennemi tank (résistant)
    tank_enemy = {
        name = "Tank Enemy",
        health = 80,
        damage = 25,
        speed = 80.0,
        score_value = 300,
        
        sprite = {
            width = 64,
            height = 64,
            texture = "enemy_tank",
            scale = 1.2
        },
        
        collider = {
            width = 60,
            height = 60
        },
        
        animation = {
            fly = {
                frames = {0, 1},
                frame_duration = 0.15
            },
            death = {
                frames = {2, 3, 4, 5, 6, 7},
                frame_duration = 0.12,
                loop = false
            }
        },
        
        movement_pattern = "straight_slow",
        
        weapon = {
            type = "spread_shot",
            fire_rate = 3.0,
            accuracy = 0.9
        },
        
        sounds = {
            spawn = "enemy_spawn_big",
            death = "enemy_death_explosion",
            shoot = "enemy_shoot_heavy"
        },
        
        drops = {
            {item = "weapon_upgrade", chance = 0.2},
            {item = "health_pack", chance = 0.3},
            {item = "score_bonus", chance = 0.5}
        }
    },
    
    -- Ennemi kamikaze
    kamikaze_enemy = {
        name = "Kamikaze",
        health = 5,
        damage = 50, -- Gros dégâts en collision
        speed = 400.0,
        score_value = 200,
        
        sprite = {
            width = 32,
            height = 32,
            texture = "enemy_kamikaze",
            scale = 1.0
        },
        
        collider = {
            width = 30,
            height = 30
        },
        
        animation = {
            fly = {
                frames = {0, 1, 2, 3},
                frame_duration = 0.05
            },
            death = {
                frames = {4, 5, 6},
                frame_duration = 0.08,
                loop = false
            }
        },
        
        movement_pattern = "homing_player", -- Fonce sur le joueur
        
        -- Pas d'arme, juste collision
        weapon = nil,
        
        sounds = {
            spawn = "kamikaze_spawn",
            death = "kamikaze_explosion",
            alert = "kamikaze_alert" -- Joué quand il lock le joueur
        },
        
        explosion_radius = 80, -- Dégâts de zone à la mort
        
        drops = {
            {item = "score_bonus", chance = 0.6}
        }
    },
    
    -- Ennemi sniper (tire de loin)
    sniper_enemy = {
        name = "Sniper",
        health = 30,
        damage = 20,
        speed = 100.0,
        score_value = 250,
        
        sprite = {
            width = 56,
            height = 48,
            texture = "enemy_sniper",
            scale = 1.0
        },
        
        collider = {
            width = 52,
            height = 44
        },
        
        animation = {
            fly = {
                frames = {0, 1},
                frame_duration = 0.2
            },
            aim = {
                frames = {2, 3},
                frame_duration = 0.1
            },
            death = {
                frames = {4, 5, 6, 7},
                frame_duration = 0.1,
                loop = false
            }
        },
        
        movement_pattern = "keep_distance", -- Reste loin du joueur
        
        weapon = {
            type = "laser",
            fire_rate = 5.0,
            accuracy = 0.95,
            charge_time = 1.5 -- Délai avant de tirer
        },
        
        sounds = {
            spawn = "enemy_spawn",
            death = "enemy_death",
            shoot = "laser_fire",
            charge = "laser_charge"
        },
        
        drops = {
            {item = "weapon_upgrade", chance = 0.15},
            {item = "score_bonus", chance = 0.4}
        }
    }
}

print("[LUA]  Enemies config loaded")

return Enemies
