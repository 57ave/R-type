-- collectables_config.lua
-- Configuration des power-ups et modules collectables

collectables_config = {
    -- ==========================================
    -- POWER-UPS (cercles simples)
    -- ==========================================
    powerups = {
        orange = {
            type = "powerup_orange",
            name = "Bomb Power-up",
            description = "Détruit tous les ennemis à l'écran",
            
            -- Sprite PNG simple (pas d'animation)
            texture_path = "assets/players/powerup_orange.png",
            rect = {x = 0, y = 0, width = 612, height = 408},
            scale = {x = 0.2, y = 0.2},  -- Scale augmenté pour ~122x82 pixels
            
            -- Mouvement
            velocity = {x = -50.0, y = 0.0},
            
            -- Effet visuel (floating)
            float_speed = 50.0,
            float_amplitude = 10.0,
            
            -- Layer de rendu
            layer = 5
        },
        
        blue = {
            type = "powerup_blue",
            name = "Shield Power-up",
            description = "Active un bouclier temporaire",
            
            -- Sprite PNG simple (pas d'animation)
            texture_path = "assets/players/powerup_blue.png",
            rect = {x = 0, y = 0, width = 612, height = 408},
            scale = {x = 0.2, y = 0.2},  -- Scale augmenté pour ~122x82 pixels
            
            -- Mouvement
            velocity = {x = -50.0, y = 0.0},
            
            -- Effet visuel (floating)
            float_speed = 50.0,
            float_amplitude = 10.0,
            
            -- Layer de rendu
            layer = 5
        }
    },
    
    -- ==========================================
    -- MODULES (armes supplémentaires animées)
    -- ==========================================
    modules = {
        laser = {
            type = "module_laser",
            name = "Laser Module",
            description = "Tire des missiles téléguidés qui suivent les ennemis",
            
            -- Sprite animé
            texture_path = "assets/players/laser_module.png",
            rect = {x = 0, y = 0, width = 34, height = 29},
            scale = {x = 2.0, y = 2.0},
            
            -- Animation
            animation = {
                frame_count = 4,
                frame_width = 34,
                frame_height = 29,
                frame_time = 0.1,
                loop = true,
                start_x = 0,
                start_y = 0
            },
            
            -- Mouvement
            velocity = {x = -50.0, y = 0.0},
            
            -- Effet visuel (floating)
            float_speed = 50.0,
            float_amplitude = 10.0,
            
            -- Layer de rendu
            layer = 5
        },
        
        spread = {
            type = "module_spread",
            name = "Spread Module",
            description = "Tire 3 projectiles en éventail",
            
            -- Sprite animé
            texture_path = "assets/players/spread_module.png",
            rect = {x = 0, y = 0, width = 30, height = 24},
            scale = {x = 2.0, y = 2.0},
            
            -- Animation
            animation = {
                frame_count = 6,
                frame_width = 30,
                frame_height = 24,
                frame_time = 0.1,
                loop = true,
                start_x = 0,
                start_y = 0
            },
            
            -- Mouvement
            velocity = {x = -50.0, y = 0.0},
            
            -- Effet visuel (floating)
            float_speed = 50.0,
            float_amplitude = 10.0,
            
            -- Layer de rendu
            layer = 5
        },
        
        wave = {
            type = "module_wave",
            name = "Wave Module",
            description = "Tire des projectiles en trajectoire sinusoïdale",
            
            -- Sprite animé
            texture_path = "assets/players/wave_module.png",
            rect = {x = 0, y = 0, width = 24, height = 19},
            scale = {x = 2.0, y = 2.0},
            
            -- Animation
            animation = {
                frame_count = 6,
                frame_width = 24,
                frame_height = 19,
                frame_time = 0.1,
                loop = true,
                start_x = 0,
                start_y = 0
            },
            
            -- Mouvement
            velocity = {x = -50.0, y = 0.0},
            
            -- Effet visuel (floating)
            float_speed = 50.0,
            float_amplitude = 10.0,
            
            -- Layer de rendu
            layer = 5
        }
    }
}

return collectables_config
