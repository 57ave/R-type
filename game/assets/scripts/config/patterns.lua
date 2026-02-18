-- ==========================================
-- R-Type Game - Movement & Fire Patterns
-- ==========================================

Patterns = {
    -- ========================================
    -- MOVEMENT PATTERNS
    -- ========================================
    movement = {
        -- Ligne droite simple
        straight = {
            description = "Move in straight line",
            -- Pas de paramètres spéciaux, utilise velocity de base
        },
        
        -- Zigzag vertical
        zigzag = {
            description = "Oscillate vertically",
            zigzag_interval = 1.0,  -- Change direction every 1 second
            -- Nécessite velocity.dy > 10 pour être détecté
        },
        
        -- Fonce vers le joueur
        kamikaze = {
            description = "Rush towards player",
            -- Nécessite velocity.dx > 250 pour être détecté
            update_interval = 0.1,  -- Update direction every 0.1s
        },
        
        -- Vague sinusoïdale
        sine_wave = {
            description = "Move in sine wave pattern",
            amplitude = 100.0,  -- Amplitude de la vague
            frequency = 2.0,    -- Fréquence (cycles par seconde)
        },
        
        -- Mouvement circulaire
        circle = {
            description = "Move in circular pattern",
            radius = 80.0,
            angular_speed = 2.0,  -- radians par seconde
        },
        
        -- Stop and go
        stop_and_go = {
            description = "Stop, wait, then continue",
            move_duration = 2.0,   -- Bouge pendant 2s
            stop_duration = 1.0,   -- S'arrête pendant 1s
        }
    },
    
    -- ========================================
    -- FIRE PATTERNS
    -- ========================================
    fire = {
        -- Tire droit vers la gauche
        straight = {
            description = "Fire straight left",
            projectile_count = 1,
            direction = {-1, 0}  -- Gauche
        },
        
        -- Tire vers le joueur
        aimed = {
            description = "Aim at player position",
            projectile_count = 1,
            lead_target = false  -- true = prédit position future du joueur
        },
        
        -- Tire en éventail (3-5 projectiles)
        spread = {
            description = "Fire in fan pattern",
            projectile_count = 3,
            angle_spread = 30.0,  -- Degrés entre chaque projectile
        },
        
        -- Tire dans toutes les directions (cercle)
        circle = {
            description = "Fire in all directions",
            projectile_count = 8,  -- 8 directions
        },
        
        -- Tire en spirale
        spiral = {
            description = "Fire in rotating spiral",
            projectile_count = 1,
            rotation_speed = 90.0,  -- Degrés par seconde
        },
        
        -- Burst rapide (3 tirs rapides)
        burst = {
            description = "Fire 3 quick shots",
            projectile_count = 3,
            burst_delay = 0.1,  -- Délai entre chaque projectile du burst
            direction = {-1, 0}
        }
    }
}

print("[LUA] ✅ Patterns config loaded")

return Patterns
