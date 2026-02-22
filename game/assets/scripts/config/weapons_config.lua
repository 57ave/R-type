-- ==========================================
-- R-Type Game - Weapons Configuration
-- Phase 7: Complete Weapons System
-- ==========================================

Weapons = {
    -- ==========================================
    -- TIR DE BASE (Niveau 0)
    -- ==========================================
    basic_shot = {
        name = "Basic Shot",
        level = 0,
        damage = 10,
        speed = 1000.0,
        fire_rate = 0.15, -- secondes entre chaque tir
        projectile_lifetime = 4.0,
        
        -- Sprite depuis r-typesheet1.png
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 245, y = 85, width = 20, height = 20},  -- Normal missile
            scale = 3.0
        },
        
        collider = {
            radius = 7.0
        },
        
        sound = "assets/vfx/shoot.ogg"
    },
    
    -- ==========================================
    -- TIR CHARGÉ - 5 NIVEAUX PROGRESSIFS
    -- ==========================================
    
    -- Niveau 1: Légèrement amélioré (0.2s de charge - maintien court)
    charge_level_1 = {
        name = "Charge Level 1",
        level = 1,
        charge_time_required = 0.2,
        damage = 25,
        speed = 950.0,
        projectile_lifetime = 4.5,
        
        sprite = {
            texture_path = "assets/players/charge_level1.png",
            rect = {x = 0, y = 0, width = 35, height = 17},  -- Dimensions réelles: 35x17 (2 frames)
            scale = 3.0
        },
        
        animation = {
            frame_count = 2,
            frame_width = 18,  -- 35px / 2 ≈ 17.5px arrondi à 18
            frame_height = 17,
            frame_time = 0.1,  -- 10 FPS
            loop = true
        },
        
        collider = {
            radius = 12.0
        },
        
        sound = "assets/vfx/shoot.ogg"
    },
    
    -- Niveau 2: Charge moyenne (0.6s de charge)
    charge_level_2 = {
        name = "Charge Level 2",
        level = 2,
        charge_time_required = 0.6,
        damage = 50,
        speed = 900.0,
        projectile_lifetime = 5.0,
        
        sprite = {
            texture_path = "assets/players/charge_level2.png",
            rect = {x = 0, y = 0, width = 68, height = 14},  -- Dimensions réelles: 68x14 (2 frames)
            scale = 3.0
        },
        
        animation = {
            frame_count = 2,
            frame_width = 34,  -- 68px / 2 = 34px
            frame_height = 14,
            frame_time = 0.1,  -- 10 FPS
            loop = true
        },
        
        collider = {
            radius = 18.0
        },
        
        sound = "assets/vfx/laser_bot.ogg"
    },
    
    -- Niveau 3: Charge forte (1.0s de charge)
    charge_level_3 = {
        name = "Charge Level 3",
        level = 3,
        charge_time_required = 1.0,
        damage = 100,
        speed = 850.0,
        projectile_lifetime = 5.5,
        
        sprite = {
            texture_path = "assets/players/charge_level3.png",
            rect = {x = 0, y = 0, width = 99, height = 15},  -- Dimensions réelles: 99x15 (2 frames)
            scale = 3.0
        },
        
        animation = {
            frame_count = 2,
            frame_width = 50,  -- 99px / 2 ≈ 49.5px arrondi à 50
            frame_height = 15,
            frame_time = 0.08,  -- 12.5 FPS (plus rapide)
            loop = true
        },
        
        collider = {
            radius = 25.0
        },
        
        sound = "assets/vfx/laser_bot.ogg"
    },
    
    -- Niveau 4: Charge très forte (1.5s de charge)
    charge_level_4 = {
        name = "Charge Level 4",
        level = 4,
        charge_time_required = 1.5,
        damage = 200,
        speed = 800.0,
        projectile_lifetime = 6.0,
        
        sprite = {
            texture_path = "assets/players/charge_level4.png",
            rect = {x = 0, y = 0, width = 99, height = 16},  -- Dimensions réelles: 99x16 (2 frames)
            scale = 3.0
        },
        
        animation = {
            frame_count = 2,
            frame_width = 50,  -- 99px / 2 ≈ 49.5px arrondi à 50
            frame_height = 16,
            frame_time = 0.08,  -- 12.5 FPS
            loop = true
        },
        
        collider = {
            radius = 32.0
        },
        
        sound = "assets/vfx/multi_laser_bot.ogg"
    },
    
    -- Niveau 5: MAXIMUM POWER! (2.0s de charge)
    charge_level_5 = {
        name = "HYPER BEAM",
        level = 5,
        charge_time_required = 2.0,
        damage = 500,
        speed = 750.0,
        projectile_lifetime = 7.0,
        
        sprite = {
            texture_path = "assets/players/charge_level5.png",
            rect = {x = 0, y = 0, width = 163, height = 18},  -- Dimensions réelles: 163x18 (2 frames)
            scale = 3.0
        },
        
        animation = {
            frame_count = 2,
            frame_width = 82,  -- 163px / 2 ≈ 81.5px arrondi à 82
            frame_height = 18,
            frame_time = 0.06,  -- 16.6 FPS (très rapide!)
            loop = true
        },
        
        collider = {
            radius = 40.0
        },
        
        sound = "assets/vfx/multi_laser_bot.ogg"
    },
    
    -- ==========================================
    -- MODULES / POWER-UPS
    -- ==========================================
    
    -- MODULE: Laser continu
    module_laser = {
        name = "Laser Module",
        type = "continuous",
        damage_per_tick = 5,
        tick_rate = 0.05,  -- 20 ticks/sec
        fire_rate = 0.0,  -- Continu
        
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 35, y = 67, width = 45, height = 7},  -- Laser beam
            scale = 3.0
        },
        
        collider = {
            radius = 10.0
        },
        
        sound = "assets/vfx/laser_bot.ogg",
        module_attachment_offset = {x = 50, y = -10}  -- Position relative au joueur
    },
    
    -- MODULE: Missiles autoguidés
    module_homing_missiles = {
        name = "Homing Missiles",
        type = "homing",
        damage = 40,
        speed = 600.0,
        turn_rate = 180.0,  -- degrés/sec
        lock_range = 500.0,
        fire_rate = 1.0,
        projectile_lifetime = 8.0,
        projectile_count = 2,  -- Tire 2 missiles
        
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 245, y = 67, width = 20, height = 7},  -- Small missile
            scale = 2.5
        },
        
        collider = {
            radius = 8.0
        },
        
        sound = "assets/vfx/shoot.ogg",
        module_attachment_offset = {x = 30, y = 20}
    },
    
    -- MODULE: Tir en éventail
    module_spread = {
        name = "Spread Shot",
        type = "spread",
        damage = 15,
        speed = 800.0,
        fire_rate = 0.25,
        projectile_count = 5,
        spread_angle = 45.0,  -- Total spread (chaque proj séparé de 45/4 degrés)
        projectile_lifetime = 4.0,
        
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 245, y = 85, width = 20, height = 20},
            scale = 1.8
        },
        
        collider = {
            radius = 6.0
        },
        
        sound = "assets/vfx/shoot.ogg",
        module_attachment_offset = {x = 50, y = 10}
    },
    
    -- MODULE: Tir en vague
    module_wave = {
        name = "Wave Beam",
        type = "wave",
        damage = 20,
        speed = 700.0,
        fire_rate = 0.3,
        projectile_lifetime = 5.0,
        wave_amplitude = 100.0,  -- Amplitude de la vague
        wave_frequency = 2.0,    -- Fréquence (cycles par seconde)
        
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 227, y = 99, width = 23, height = 7},
            scale = 2.2
        },
        
        collider = {
            radius = 9.0
        },
        
        sound = "assets/vfx/laser_bot.ogg",
        module_attachment_offset = {x = 50, y = -20}
    },
    
    -- ==========================================
    -- POWER-UP COLLECTABLES (visuels des modules à ramasser)
    -- ==========================================
    powerup_laser = {
        name = "Laser Power-Up",
        module_type = "module_laser",
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 67, y = 50, width = 31, height = 15},  -- Pickup sprite
            scale = 2.0
        },
        collider = {
            radius = 20.0
        }
    },
    
    powerup_missiles = {
        name = "Missiles Power-Up",
        module_type = "module_homing_missiles",
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 99, y = 50, width = 31, height = 15},
            scale = 2.0
        },
        collider = {
            radius = 20.0
        }
    },
    
    powerup_spread = {
        name = "Spread Power-Up",
        module_type = "module_spread",
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 131, y = 50, width = 31, height = 15},
            scale = 2.0
        },
        collider = {
            radius = 20.0
        }
    },
    
    powerup_wave = {
        name = "Wave Power-Up",
        module_type = "module_wave",
        sprite = {
            texture_path = "assets/players/r-typesheet1.png",
            rect = {x = 163, y = 50, width = 31, height = 15},
            scale = 2.0
        },
        collider = {
            radius = 20.0
        }
    },

    -- ==========================================
    -- ANIMATION DE CHARGE
    -- ==========================================
    charge_animation = {
        -- Texture source (nouveau fichier cropé)
        texture_path = "assets/players/animation_charge.png",
        
        -- Position relative au joueur (DEVANT le vaisseau)
        offset = {
            x = 80.0,   -- DEVANT le vaisseau (à droite)
            y = 0.0     -- À la même hauteur (centre)
        },
        
        -- Échelle de l'animation
        scale = {
            x = 2.0,
            y = 2.0
        },
        
        -- Layer de rendu
        layer = 10,  -- Au-dessus du joueur
        
        -- Frames de l'animation (8 frames horizontales: 32x32 chacune)
        frames = {
            -- Frame 0
            { x = 0,   y = 0, width = 32, height = 32 },
            -- Frame 1
            { x = 32,  y = 0, width = 32, height = 32 },
            -- Frame 2
            { x = 64,  y = 0, width = 32, height = 32 },
            -- Frame 3
            { x = 96,  y = 0, width = 32, height = 32 },
            -- Frame 4
            { x = 128, y = 0, width = 32, height = 32 },
            -- Frame 5
            { x = 160, y = 0, width = 32, height = 32 },
            -- Frame 6
            { x = 192, y = 0, width = 32, height = 32 },
            -- Frame 7
            { x = 224, y = 0, width = 32, height = 32 }
        },
        
        -- Mapping des niveaux de charge aux frames
            level_frames = {
            [0] = { 0, 1 },  -- Niveau 0: oscillation entre frames 0-1
            [1] = { 2 },     -- Niveau 1: frame 2 fixe
            [2] = { 3 },     -- Niveau 2: frame 3 fixe
            [3] = { 4 },     -- Niveau 3: frame 4 fixe
            [4] = { 5 },     -- Niveau 4: frame 5 fixe
            [5] = { 6, 7 }   -- Niveau 5: oscillation entre frames 6-7
        },
        
        -- Timing pour les oscillations
        oscillation_speed = 10.0  -- Vitesse d'oscillation (Hz)
    }
}

-- Charge timings (pour faciliter l'accès)
-- 0.0s = basic_shot (tir rapide/spam)
-- 0.2s = charge_level_1 (maintien court)
-- 0.6s = charge_level_2
-- 1.0s = charge_level_3
-- 1.5s = charge_level_4
-- 2.0s = charge_level_5 (maintien long)
Weapons.charge_timings = {0.0, 0.2, 0.6, 1.0, 1.5, 2.0}

print("[LUA]  Weapons config loaded (basic + 5 charge levels + 4 modules)")

return Weapons
