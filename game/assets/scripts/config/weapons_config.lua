-- ==========================================
-- R-Type Game - Weapons Configuration
-- ==========================================

Weapons = {
    -- Tir de base
    basic_shot = {
        name = "Basic Shot",
        damage = 10,
        speed = 600.0,
        fire_rate = 0.2, -- secondes entre chaque tir
        projectile_lifetime = 5.0,
        
        sprite = {
            width = 32,
            height = 8,
            texture = "projectile_basic"
        },
        
        collider = {
            width = 30,
            height = 6
        },
        
        sound = "shoot_basic"
    },
    
    -- Tir chargé
    charge_shot = {
        name = "Charge Shot",
        damage = 50,
        speed = 500.0,
        charge_time = 2.0, -- temps de charge requis
        projectile_lifetime = 6.0,
        
        sprite = {
            width = 64,
            height = 32,
            texture = "projectile_charge"
        },
        
        collider = {
            width = 60,
            height = 28
        },
        
        animation = {
            frames = {0, 1, 2, 3, 4},
            frame_duration = 0.1
        },
        
        sound = "shoot_charge",
        
        -- Particules et effets
        trail = {
            enabled = true,
            color = {255, 100, 0, 255},
            particle_count = 20
        }
    },
    
    -- Laser
    laser = {
        name = "Laser Beam",
        damage_per_second = 80,
        max_duration = 3.0,
        width = 16,
        max_length = 800,
        
        sprite = {
            width = 800,
            height = 16,
            texture = "laser_beam"
        },
        
        sound = "laser_loop",
        
        energy_cost = 30 -- par seconde
    },
    
    -- Missiles
    homing_missile = {
        name = "Homing Missile",
        damage = 40,
        speed = 400.0,
        turn_rate = 180.0, -- degrés par seconde
        lock_range = 500.0,
        fire_rate = 1.0,
        projectile_lifetime = 8.0,
        
        sprite = {
            width = 48,
            height = 24,
            texture = "missile"
        },
        
        collider = {
            width = 44,
            height = 20
        },
        
        sound = "missile_launch",
        
        trail = {
            enabled = true,
            color = {200, 200, 255, 255},
            particle_count = 15
        }
    },
    
    -- Spread shot
    spread_shot = {
        name = "Spread Shot",
        damage = 8,
        speed = 550.0,
        fire_rate = 0.3,
        projectile_count = 5,
        spread_angle = 30.0, -- degrés
        projectile_lifetime = 4.0,
        
        sprite = {
            width = 24,
            height = 12,
            texture = "projectile_spread"
        },
        
        collider = {
            width = 22,
            height = 10
        },
        
        sound = "shoot_spread"
    }
}

print("[LUA] ✅ Weapons config loaded")

return Weapons
