-- ==========================================
-- R-Type Server Configuration
-- All gameplay values used by the authoritative server
-- ==========================================

ServerConfig = {

    -- ==========================================
    -- PLAYER
    -- ==========================================
    player = {
        speed = 500.0,
        max_health = 100,
        spawn_x = 100.0,
        spawn_y_start = 200.0,
        spawn_y_offset = 200.0,  -- vertical offset per player index
        boundary = {
            min_x = 0,
            min_y = 0,
            max_x = 1820,
            max_y = 1030,
        },
    },

    -- ==========================================
    -- ENEMIES
    -- ==========================================
    enemies = {
        -- type 0: Bug
        bug = {
            type_id = 0,
            health = 10,
            vx = -400.0,
            vy = 0.0,
            fire_pattern = 0,    -- straight
            fire_rate = 2.0,
            collision_damage = 20,
            score = 100,
        },
        -- type 1: Fighter / Bat
        fighter = {
            type_id = 1,
            health = 10,
            vx = -350.0,
            vy = 80.0,
            fire_pattern = 2,    -- circle
            fire_rate = 1.5,
            collision_damage = 20,
            zigzag_interval = 1.0,
            boundary_top = 50.0,
            boundary_bottom = 1000.0,
            score = 150,
        },
        -- type 2: Kamikaze
        kamikaze = {
            type_id = 2,
            health = 20,
            vx = -500.0,
            vy = 0.0,
            fire_pattern = 255,  -- no fire
            fire_rate = 999.0,
            tracking_speed = 500.0,
            collision_damage = 20,
            score = 100,
        },
        -- Spawn settings
        spawn_x = 1920.0,
        spawn_y_min = 100.0,
        spawn_y_range = 880,
        fire_timer_base = 1.0,
        fire_timer_random_range = 200,  -- /100 added to base
    },

    -- ==========================================
    -- BOSSES (common movement settings)
    -- ==========================================
    bosses = {
        spawn_x = 1920.0,
        spawn_y = 400.0,
        stop_x = 1500.0,           -- boss stops moving at this x
        bob_speed = 1.5,           -- sin wave speed for up/down
        bob_amplitude = 100.0,     -- sin wave amplitude
        boundary_top = 50.0,
        boundary_bottom = 900.0,
        score = 500,
        -- Per-boss collision damage to player
        collision_damage_to_player = 30,
        collision_damage_from_player = 20,
    },

    -- ==========================================
    -- PROJECTILES
    -- ==========================================
    projectiles = {
        player = {
            normal_speed = 800.0,
            charged_speed = 1500.0,
            base_damage = 10,
            charge_damage_multiplier = 10,  -- damage = chargeLevel * this
            fire_cooldown_normal = 0.15,
            fire_cooldown_charged = 0.3,
            spawn_offset_x = 50.0,
            spawn_offset_y = 10.0,
        },
        enemy = {
            speed_multiplier = 1.5,   -- projectile speed = |enemy.vx| * this
            min_speed = 400.0,
            circle_count = 8,         -- number of projectiles in circle pattern
            circle_speed_factor = 0.8,
            spread_angle = 0.26,      -- ~15 degrees
            spawn_offset_x = -40.0,
        },
        -- Enemy missile damage to player
        missile_damage = 10,
    },

    -- ==========================================
    -- MODULES (player weapon modules)
    -- ==========================================
    modules = {
        fire_cooldown = 0.2,
        base_speed = 800.0,
        -- Homing (laser module, type 1)
        homing = {
            speed = 500.0,
            detection_radius = 600.0,
            turn_rate = 5.0,
            projectile_type = 3,
        },
        -- Spread (type 3)
        spread = {
            angles = {-0.2617, 0.0, 0.2617},  -- -15°, 0°, +15°
            projectile_type = 4,
        },
        -- Wave (type 4)
        wave = {
            amplitude = 60.0,
            frequency = 4.0,
            projectile_type = 5,
        },
        -- Spawn settings (collectible modules in world)
        spawn_vx = -100.0,
    },

    -- ==========================================
    -- POWERUPS
    -- ==========================================
    powerups = {
        spawn_vx = -150.0,
        spawn_x = 1920.0,
        spawn_y_min = 100.0,
        spawn_y_range = 880,
        -- Orange bomb
        orange = {
            boss_damage_fraction = 0.25,  -- 25% of boss max HP
        },
        -- Blue shield
        blue = {
            duration = 10.0,
        },
    },

    -- ==========================================
    -- EXPLOSIONS
    -- ==========================================
    explosions = {
        lifetime = 0.5,
    },

    -- ==========================================
    -- COLLISIONS
    -- ==========================================
    collisions = {
        hitbox_size = 50.0,
        -- Out of bounds margins
        oob_margin = 100.0,
        screen_width = 1920.0,
        screen_height = 1080.0,
    },

    -- ==========================================
    -- LEVELS
    -- ==========================================
    levels = {
        max_level = 3,
        -- Level 1: First Contact
        [1] = {
            name = "First Contact",
            enemy_types = {0},
            module_types = {3, 4},
            enemy_interval = 2.5,
            powerup_interval = 15.0,
            module_interval = 25.0,
            max_enemies = 8,
            stop_spawning_at_boss = true,
            waves = {
                {time = 3.0,  groups = {{type = 0, count = 3, interval = 1.5}}},
                {time = 15.0, groups = {{type = 0, count = 5, interval = 1.0}}},
                {time = 30.0, groups = {{type = 0, count = 6, interval = 0.8}}},
                {time = 50.0, groups = {{type = 0, count = 8, interval = 0.6}}},
                {time = 70.0, groups = {{type = 0, count = 10, interval = 0.5}}},
            },
            boss = {
                enemy_type = 3,
                health = 1000,
                speed = 80.0,
                fire_rate = 2.0,
                fire_pattern = 0,
                spawn_time = 90.0,
            },
        },
        -- Level 2: Rising Threat
        [2] = {
            name = "Rising Threat",
            enemy_types = {0, 1},
            module_types = {3, 4},
            enemy_interval = 2.0,
            powerup_interval = 12.0,
            module_interval = 22.0,
            max_enemies = 12,
            stop_spawning_at_boss = true,
            waves = {
                {time = 3.0,  groups = {{type = 0, count = 3, interval = 1.2}, {type = 1, count = 2, interval = 1.5}}},
                {time = 18.0, groups = {{type = 0, count = 4, interval = 0.8}, {type = 1, count = 3, interval = 1.0}}},
                {time = 35.0, groups = {{type = 1, count = 5, interval = 0.7}, {type = 0, count = 3, interval = 1.0}}},
                {time = 55.0, groups = {{type = 0, count = 6, interval = 0.5}, {type = 1, count = 4, interval = 0.6}}},
                {time = 75.0, groups = {{type = 0, count = 8, interval = 0.4}, {type = 1, count = 5, interval = 0.5}}},
            },
            boss = {
                enemy_type = 4,
                health = 2000,
                speed = 60.0,
                fire_rate = 1.5,
                fire_pattern = 2,
                spawn_time = 95.0,
            },
        },
        -- Level 3: Final Assault
        [3] = {
            name = "Final Assault",
            enemy_types = {0, 1, 2},
            module_types = {1, 3, 4},
            enemy_interval = 1.5,
            powerup_interval = 10.0,
            module_interval = 20.0,
            max_enemies = 15,
            stop_spawning_at_boss = true,
            waves = {
                {time = 3.0,  groups = {{type = 0, count = 4, interval = 0.8}, {type = 1, count = 3, interval = 1.0}, {type = 2, count = 2, interval = 1.2}}},
                {time = 18.0, groups = {{type = 2, count = 5, interval = 0.6}, {type = 0, count = 3, interval = 0.8}}},
                {time = 35.0, groups = {{type = 0, count = 5, interval = 0.5}, {type = 1, count = 4, interval = 0.6}, {type = 2, count = 3, interval = 0.7}}},
                {time = 55.0, groups = {{type = 0, count = 8, interval = 0.3}, {type = 1, count = 5, interval = 0.4}, {type = 2, count = 4, interval = 0.5}}},
                {time = 75.0, groups = {{type = 0, count = 10, interval = 0.3}, {type = 1, count = 6, interval = 0.4}, {type = 2, count = 5, interval = 0.4}}},
            },
            boss = {
                enemy_type = 5,
                health = 3000,
                speed = 100.0,
                fire_rate = 1.0,
                fire_pattern = 3,
                spawn_time = 95.0,
            },
        },
    },

    -- ==========================================
    -- SERVER SETTINGS
    -- ⚠️  SEUL ENDROIT À MODIFIER pour l'IP et le port.
    -- Le client lit aussi ces valeurs via game_config.lua qui importe ce fichier.
    --
    -- server_ip  : IP d'écoute du serveur ET IP de connexion du client
    --   "127.0.0.1"    → local (même machine)
    --   "192.168.1.x"  → réseau local (LAN)
    --   "0.0.0.0"      → écoute sur toutes les interfaces (serveur dédié)
    -- ==========================================
    server = {
        server_ip = "127.0.0.1",
        port = 12345,
        tick_rate = 60,        -- simulation FPS
        snapshot_rate = 30,    -- network snapshot FPS
        min_players_to_start = 2,
        max_player_ships = 5,  -- number of different ship colors
    },
}

print("[LUA] ✅ Server config loaded")

return ServerConfig
