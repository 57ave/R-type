-- ==========================================
-- R-Type Game - Enemies Configuration (Simplified)
-- ==========================================

EnemiesSimple = {
    -- Basic enemy (basic_enemie.png: 255x29, 8 frames ≈ 32px per frame)
    bug = {
        name = "BasicEnemy",
        health = 10,
        damage = 20,  -- Collision damage (5 hits to kill player with 100 HP)
        speed = 350.0,  -- Augmenté de 200 à 350
        score = 50,
        
        sprite = {
            path = "assets/enemies/basic_enemie.png",
            rect = {0, 0, 33, 29},  -- Using 32px (8*32=256, close to 255)
            scale = {2.0, 2.0}
        },
        
        animation = {
            frame_count = 8,
            frame_width = 33,
            frame_height = 29,
            frame_time = 0.1
        },
        
        collider = {
            width = 33,
            height = 29
        },
        
        -- Pattern de mouvement
        movement = {
            type = "straight",
            velocity = {-400, 0}  -- Augmenté de -250 à -400
        },
        
        -- Tir
        weapon = {
            enabled = true,
            fire_rate = 2.0,
            projectile_speed = 600,  -- Plus rapide que l'ennemi (350) - Augmenté de 400 à 600
            projectile_damage = 10,  -- Missile damage (10 hits to kill player with 100 HP)
            pattern = "straight"  -- Tire droit vers la gauche
        }
    },
    
    -- Bat enemy (BatEnemies.png: 255x29, 5 frames = 51px per frame)
    fighter = {
        name = "BatEnemy",
        health = 10,
        damage = 20,  -- Collision damage (5 hits to kill player with 100 HP)
        speed = 250.0,  -- Augmenté de 150 à 250
        score = 150,
        
        sprite = {
            path = "assets/enemies/BatEnemies.png",
            rect = {0, 0, 16, 13},  -- 255/5 = 51 exactly
            scale = {2.0, 2.0}
        },
        
        animation = {
            frame_count = 5,
            frame_width = 16,
            frame_height = 13,
            frame_time = 0.12
        },
        
        collider = {
            width = 16,
            height = 13
        },
        
        movement = {
            type = "zigzag",
            velocity = {-350, 80}  -- Augmenté de -200 à -350, et dy de 50 à 80
        },
        
        weapon = {
            enabled = true,
            fire_rate = 1.5,
            projectile_speed = 550,  -- Plus rapide que l'ennemi (350) - Augmenté de 500 à 550
            projectile_damage = 10,  -- Missile damage (10 hits to kill player with 100 HP)
            pattern = "circle"  -- Tire dans toutes les directions (8 projectiles)
        }
    },
    
    -- Kamikaze enemy (kamikaze_enemies.png: 205x18, 12 frames)
    tank = {
        name = "Kamikaze",
        health = 20,
        damage = 20,  -- Collision damage (5 hits to kill player with 100 HP)
        speed = 500.0,  -- Fast rush towards player (must be >= 450 to trigger kamikaze pattern)
        score = 100,
        
        sprite = {
            path = "assets/enemies/kamikaze_enemies.png",
            rect = {0, 0, 17, 18},  -- 205/12 ≈ 17px wide, 18px tall
            scale = {2.0, 2.0}
        },
        
        animation = {
            frame_count = 12,
            frame_width = 17,
            frame_height = 18,
            frame_time = 0.08
        },
        
        collider = {
            width = 17,
            height = 18
        },
        
        movement = {
            type = "kamikaze",
            velocity = {-500, 0}  -- Must be >= 450 to trigger kamikaze pattern
        },
        
        weapon = {
            enabled = false  -- Kamikaze doesn't shoot, just rams
        }
    },
    
    -- Petit ennemi en formation (r-typesheet5: 533x36, frames de 36x36)
    drone = {
        name = "Drone",
        health = 15,
        damage = 20,  -- Collision damage (5 hits to kill player with 100 HP)
        speed = 320.0,  -- Augmenté de 180 à 320
        score = 100,
        
        sprite = {
            path = "assets/enemies/r-typesheet20.png",
            rect = {0, 0, 36, 36},
            scale = {2.0, 2.0}
        },
        
        animation = {
            frame_count = 4,
            frame_width = 36,
            frame_height = 36,  -- ← FIX: Ajouter hauteur
            frame_time = 0.08
        },
        
        collider = {
            width = 36,
            height = 36
        },
        
        movement = {
            type = "formation",
            velocity = {-400, 0},  -- Augmenté de -250 à -400 - Drone rapide
            formation_offset = {0, 0}
        },
        
        weapon = {
            enabled = true,
            fire_rate = 1.8,
            projectile_speed = 650,  -- Plus rapide que l'ennemi (400) - Augmenté de 450 à 650
            projectile_damage = 10,  -- Missile damage (10 hits to kill player with 100 HP)
            pattern = "aimed"  -- Tire vers le joueur (tracking)
        }
    },
    
    -- Config des projectiles ennemis
    enemy_projectiles = {
        basic = {
            sprite = {
                path = "assets/enemies/enemy_bullets.png",
                rect = {0, 0, 16, 16},
                scale = {2.0, 2.0}
            },
            collider = {
                width = 16,
                height = 16
            },
            lifetime = 3.0
        }
    }
}



return EnemiesSimple
