-- ==========================================
-- R-Type Game - Bosses Configuration
-- ==========================================

Bosses = {
    -- Boss 1 : The Guardian
    guardian_boss = {
        name = "The Guardian",
        health = 2000,
        damage = 30,
        speed = 50.0,
        score_value = 10000,
        
        sprite = {
            width = 256,
            height = 256,
            texture = "boss_guardian",
            scale = 1.5
        },
        
        collider = {
            width = 220,
            height = 220
        },
        
        animation = {
            idle = {
                frames = {0, 1, 2, 3},
                frame_duration = 0.15
            },
            attack = {
                frames = {4, 5, 6, 7, 8},
                frame_duration = 0.1
            },
            hurt = {
                frames = {9, 10},
                frame_duration = 0.1
            },
            death = {
                frames = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20},
                frame_duration = 0.2,
                loop = false
            }
        },
        
        -- Position de spawn
        spawn_position = {
            x = 1600,
            y = 540
        },
        
        -- Phases de combat
        phases = {
            {
                health_threshold = 100, -- % de santé
                pattern = "pattern_1",
                movement = "circle_pattern",
                fire_rate = 3.0
            },
            {
                health_threshold = 60,
                pattern = "pattern_2",
                movement = "aggressive_chase",
                fire_rate = 2.0
            },
            {
                health_threshold = 30,
                pattern = "pattern_3_rage",
                movement = "erratic",
                fire_rate = 1.0
            }
        },
        
        -- Patterns d'attaque
        attack_patterns = {
            pattern_1 = {
                name = "Spiral Shot",
                weapons = {"basic_shot"},
                projectile_count = 12,
                rotation_speed = 45.0,
                duration = 5.0
            },
            pattern_2 = {
                name = "Spread + Laser",
                weapons = {"spread_shot", "laser"},
                alternating = true,
                interval = 2.0
            },
            pattern_3_rage = {
                name = "Bullet Hell",
                weapons = {"basic_shot", "spread_shot", "homing_missile"},
                simultaneous = true,
                projectile_count = 24,
                chaos_mode = true
            }
        },
        
        -- Points faibles (weakpoints)
        weak_points = {
            {
                name = "Core",
                position = {x = 0, y = 0}, -- Relatif au boss
                radius = 40,
                damage_multiplier = 2.0,
                health = 500
            },
            {
                name = "Left Turret",
                position = {x = -80, y = -60},
                radius = 30,
                damage_multiplier = 1.5,
                health = 300
            },
            {
                name = "Right Turret",
                position = {x = 80, y = -60},
                radius = 30,
                damage_multiplier = 1.5,
                health = 300
            }
        },
        
        -- Sons
        sounds = {
            spawn = "boss_guardian_spawn",
            attack = "boss_attack",
            hurt = "boss_hurt",
            death = "boss_death_explosion",
            phase_transition = "boss_phase_change",
            music = "boss_guardian_theme"
        },
        
        -- Drops (garantis après victoire)
        drops = {
            {item = "weapon_upgrade", chance = 1.0},
            {item = "health_pack", chance = 1.0},
            {item = "score_bonus_large", chance = 1.0}
        }
    },
    
    -- Boss 2 : The Destroyer
    destroyer_boss = {
        name = "The Destroyer",
        health = 3500,
        damage = 40,
        speed = 80.0,
        score_value = 20000,
        
        sprite = {
            width = 320,
            height = 240,
            texture = "boss_destroyer",
            scale = 1.5
        },
        
        collider = {
            width = 280,
            height = 200
        },
        
        animation = {
            idle = {
                frames = {0, 1, 2, 3, 4},
                frame_duration = 0.12
            },
            attack = {
                frames = {5, 6, 7, 8, 9, 10},
                frame_duration = 0.08
            },
            charge = {
                frames = {11, 12, 13, 14},
                frame_duration = 0.15
            },
            hurt = {
                frames = {15, 16},
                frame_duration = 0.1
            },
            death = {
                frames = {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
                frame_duration = 0.18,
                loop = false
            }
        },
        
        spawn_position = {
            x = 1700,
            y = 540
        },
        
        phases = {
            {
                health_threshold = 100,
                pattern = "destroyer_pattern_1",
                movement = "floating",
                fire_rate = 2.5
            },
            {
                health_threshold = 70,
                pattern = "destroyer_pattern_2",
                movement = "aggressive",
                fire_rate = 1.8,
                spawns_minions = true
            },
            {
                health_threshold = 40,
                pattern = "destroyer_pattern_3",
                movement = "berserker",
                fire_rate = 1.0,
                invulnerable_phases = true -- Phases d'invincibilité
            },
            {
                health_threshold = 15,
                pattern = "destroyer_final",
                movement = "desperate",
                fire_rate = 0.5,
                mega_attacks = true
            }
        },
        
        attack_patterns = {
            destroyer_pattern_1 = {
                name = "Missile Barrage",
                weapons = {"homing_missile"},
                projectile_count = 8,
                interval = 0.3
            },
            destroyer_pattern_2 = {
                name = "Laser Grid",
                weapons = {"laser"},
                laser_count = 4,
                sweep_pattern = true,
                duration = 6.0
            },
            destroyer_pattern_3 = {
                name = "Chaos Storm",
                weapons = {"basic_shot", "spread_shot", "homing_missile", "laser"},
                all_at_once = true,
                duration = 8.0
            },
            destroyer_final = {
                name = "Annihilation",
                weapons = {"mega_laser"},
                screen_wide = true,
                charge_time = 3.0,
                damage = 100
            }
        },
        
        -- Invoque des ennemis
        minion_spawns = {
            wave_1 = {
                enemies = {"basic_enemy", "fast_enemy"},
                count = 5,
                interval = 2.0
            },
            wave_2 = {
                enemies = {"tank_enemy", "kamikaze_enemy"},
                count = 3,
                interval = 3.0
            }
        },
        
        weak_points = {
            {
                name = "Main Core",
                position = {x = 0, y = 0},
                radius = 50,
                damage_multiplier = 3.0,
                health = 1000,
                vulnerable_only_when_open = true
            },
            {
                name = "Left Engine",
                position = {x = -100, y = 80},
                radius = 35,
                damage_multiplier = 2.0,
                health = 500
            },
            {
                name = "Right Engine",
                position = {x = 100, y = 80},
                radius = 35,
                damage_multiplier = 2.0,
                health = 500
            },
            {
                name = "Top Cannon",
                position = {x = 0, y = -80},
                radius = 40,
                damage_multiplier = 1.8,
                health = 600
            }
        },
        
        sounds = {
            spawn = "boss_destroyer_spawn",
            attack = "boss_destroyer_attack",
            charge = "boss_charge_mega",
            hurt = "boss_hurt_heavy",
            death = "boss_death_massive",
            phase_transition = "boss_phase_change_heavy",
            music = "boss_destroyer_theme"
        },
        
        drops = {
            {item = "weapon_upgrade_max", chance = 1.0},
            {item = "health_full", chance = 1.0},
            {item = "score_bonus_mega", chance = 1.0},
            {item = "secret_weapon", chance = 0.5}
        }
    }
}

print("[LUA]  Bosses config loaded")

return Bosses
