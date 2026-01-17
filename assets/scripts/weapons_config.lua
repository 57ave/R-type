-- ============================================================================
-- WEAPONS CONFIGURATION
-- All weapon types and their properties - fully data-driven
-- ============================================================================

WeaponsConfig = {
    -- ========================================================================
    -- PLAYER WEAPONS
    -- ========================================================================

    -- Basic single shot (starting weapon)
    single_shot = {
        name = "Vulcan",
        description = "Basic rapid-fire weapon",
        category = "primary",

        -- Fire properties
        fireRate = 0.15,           -- Seconds between shots
        projectileSpeed = 1000,
        damage = 10,
        projectileCount = 1,
        spreadAngle = 0,

        -- Charge capability
        canCharge = true,
        minChargeTime = 0.3,
        maxChargeTime = 1.5,
        chargelevels = {
            { threshold = 0.3, damage = 25,  speed = 800,  size = 1.2 },
            { threshold = 0.5, damage = 50,  speed = 750,  size = 1.5 },
            { threshold = 0.8, damage = 100, speed = 700,  size = 2.0 },
            { threshold = 1.1, damage = 150, speed = 650,  size = 2.5 },
            { threshold = 1.5, damage = 250, speed = 600,  size = 3.0 }
        },

        -- Projectile visuals
            projectile = {
            texture = "players/r-typesheet42.png",
            normalRect = { x = 245, y = 85, w = 20, h = 20 },
            -- Per-level charged rects (index = charge level 1..5)
            chargedRects = {
                { x = 233, y = 100, w = 15, h = 15 },
                { x = 202, y = 117, w = 31, h = 15 },
                { x = 170, y = 135, w = 47, h = 15 },
                { x = 138, y = 155, w = 63, h = 15 },
                { x = 105, y = 170, w = 79, h = 17 }
            },
            scale = 3.0,
            animated = false
        },

        -- Upgrade levels (collected power-ups increase level)
        levels = {
            [1] = { damage = 10,  fireRate = 0.15, projectileCount = 1 },
            [2] = { damage = 12,  fireRate = 0.13, projectileCount = 1 },
            [3] = { damage = 15,  fireRate = 0.11, projectileCount = 1 },
            [4] = { damage = 18,  fireRate = 0.09, projectileCount = 1 },
            [5] = { damage = 22,  fireRate = 0.07, projectileCount = 1 }
        },

        sound = "shoot"
    },

    -- Double shot
    double_shot = {
        name = "Twin Vulcan",
        description = "Fires two parallel shots",
        category = "primary",

        fireRate = 0.18,
        projectileSpeed = 1000,
        damage = 8,
        projectileCount = 2,
        spreadAngle = 0,

        -- Offsets for multiple projectiles
        projectileOffsets = {
            { x = 0, y = -15 },
            { x = 0, y = 15 }
        },

        canCharge = true,
        minChargeTime = 0.3,
        maxChargeTime = 1.5,
        chargelevels = {
            { threshold = 0.3, damage = 20,  speed = 800,  size = 1.2 },
            { threshold = 0.5, damage = 40,  speed = 750,  size = 1.5 },
            { threshold = 0.8, damage = 80,  speed = 700,  size = 2.0 },
            { threshold = 1.1, damage = 120, speed = 650,  size = 2.5 },
            { threshold = 1.5, damage = 200, speed = 600,  size = 3.0 }
        },

        projectile = {
            texture = "players/r-typesheet42.png",
            normalRect = { x = 245, y = 85, w = 20, h = 20 },
            chargedRects = {
                { x = 233, y = 100, w = 15, h = 15 },
                { x = 202, y = 117, w = 31, h = 15 },
                { x = 170, y = 135, w = 47, h = 15 },
                { x = 138, y = 155, w = 63, h = 15 },
                { x = 105, y = 170, w = 79, h = 17 }
            },
            scale = 2.5,
            animated = false
        },

        levels = {
            [1] = { damage = 8,   fireRate = 0.18, projectileCount = 2 },
            [2] = { damage = 10,  fireRate = 0.16, projectileCount = 2 },
            [3] = { damage = 12,  fireRate = 0.14, projectileCount = 2 },
            [4] = { damage = 14,  fireRate = 0.12, projectileCount = 2 },
            [5] = { damage = 18,  fireRate = 0.10, projectileCount = 2 }
        },

        sound = "shoot"
    },

    -- Spread shot
    spread_shot = {
        name = "Spread Gun",
        description = "Fires in a wide arc",
        category = "primary",

        fireRate = 0.25,
        projectileSpeed = 900,
        damage = 6,
        projectileCount = 3,
        spreadAngle = 30,  -- Total spread angle

        canCharge = false,

        projectile = {
            texture = "players/r-typesheet42.png",
            normalRect = { x = 245, y = 85, w = 20, h = 20 },
            scale = 2.0,
            animated = false
        },

        levels = {
            [1] = { damage = 6,  fireRate = 0.25, projectileCount = 3, spreadAngle = 30 },
            [2] = { damage = 7,  fireRate = 0.23, projectileCount = 4, spreadAngle = 40 },
            [3] = { damage = 8,  fireRate = 0.21, projectileCount = 5, spreadAngle = 50 },
            [4] = { damage = 9,  fireRate = 0.19, projectileCount = 6, spreadAngle = 60 },
            [5] = { damage = 11, fireRate = 0.17, projectileCount = 7, spreadAngle = 70 }
        },

        sound = "shoot"
    },

    -- Laser beam
    laser = {
        name = "Laser",
        description = "Powerful penetrating beam",
        category = "primary",

        fireRate = 0.4,
        projectileSpeed = 1500,
        damage = 25,
        projectileCount = 1,
        spreadAngle = 0,

        -- Special properties
        piercing = true,        -- Goes through enemies
        maxPierceCount = 3,     -- Max enemies to pierce

        canCharge = true,
        minChargeTime = 0.5,
        maxChargeTime = 2.0,
        chargelevels = {
            { threshold = 0.5, damage = 50,  speed = 1200, pierceCount = 5 },
            { threshold = 1.0, damage = 100, speed = 1000, pierceCount = 8 },
            { threshold = 1.5, damage = 200, speed = 800,  pierceCount = -1 },  -- -1 = infinite
            { threshold = 2.0, damage = 350, speed = 700,  pierceCount = -1 }
        },

        projectile = {
            texture = "players/r-typesheet42.png",
            normalRect = { x = 1, y = 51, w = 64, h = 14 },
            chargedRect = { x = 1, y = 67, w = 96, h = 18 },
            scale = 2.0,
            animated = true,
            frameCount = 2,
            frameTime = 0.05
        },

        levels = {
            [1] = { damage = 25, fireRate = 0.40, maxPierceCount = 3 },
            [2] = { damage = 30, fireRate = 0.36, maxPierceCount = 4 },
            [3] = { damage = 35, fireRate = 0.32, maxPierceCount = 5 },
            [4] = { damage = 42, fireRate = 0.28, maxPierceCount = 6 },
            [5] = { damage = 50, fireRate = 0.24, maxPierceCount = 8 }
        },

        sound = "laser"
    },

    -- Homing missiles
    homing_missile = {
        name = "Homing Missile",
        description = "Seeks out enemies",
        category = "secondary",

        fireRate = 0.6,
        projectileSpeed = 600,
        damage = 35,
        projectileCount = 1,
        spreadAngle = 0,

        -- Homing properties
        homing = true,
        homingStrength = 5.0,    -- Turn rate
        homingRange = 500,       -- Detection range
        homingDelay = 0.2,       -- Time before homing activates

        canCharge = false,

        projectile = {
            texture = "players/r-typesheet42.png",
            normalRect = { x = 103, y = 169, w = 16, h = 8 },
            scale = 3.0,
            animated = true,
            frameCount = 2,
            frameTime = 0.1
        },

        levels = {
            [1] = { damage = 35, fireRate = 0.60, projectileCount = 1 },
            [2] = { damage = 40, fireRate = 0.55, projectileCount = 1 },
            [3] = { damage = 45, fireRate = 0.50, projectileCount = 2 },
            [4] = { damage = 50, fireRate = 0.45, projectileCount = 2 },
            [5] = { damage = 60, fireRate = 0.40, projectileCount = 3 }
        },

        sound = "missile"
    },

    -- Wave beam (bounces off walls)
    wave_beam = {
        name = "Wave Beam",
        description = "Bounces off surfaces",
        category = "primary",

        fireRate = 0.3,
        projectileSpeed = 800,
        damage = 15,
        projectileCount = 1,
        spreadAngle = 0,

        -- Bounce properties
        bouncing = true,
        maxBounces = 3,
        bounceAngleVariation = 10,  -- Slight randomness on bounce

        canCharge = true,
        minChargeTime = 0.3,
        maxChargeTime = 1.0,
        chargelevels = {
            { threshold = 0.3, damage = 30,  maxBounces = 4 },
            { threshold = 0.6, damage = 50,  maxBounces = 5 },
            { threshold = 1.0, damage = 80,  maxBounces = 7 }
        },

        projectile = {
            texture = "players/r-typesheet42.png",
            normalRect = { x = 167, y = 102, w = 20, h = 20 },
            scale = 2.5,
            animated = true,
            frameCount = 4,
            frameTime = 0.08
        },

        levels = {
            [1] = { damage = 15, fireRate = 0.30, maxBounces = 3 },
            [2] = { damage = 18, fireRate = 0.28, maxBounces = 4 },
            [3] = { damage = 21, fireRate = 0.26, maxBounces = 5 },
            [4] = { damage = 25, fireRate = 0.24, maxBounces = 6 },
            [5] = { damage = 30, fireRate = 0.22, maxBounces = 8 }
        },

        sound = "wave"
    },

    -- ========================================================================
    -- ENEMY WEAPONS
    -- ========================================================================

    enemy_bullet = {
        name = "Enemy Shot",
        category = "enemy",

        fireRate = 1.5,
        projectileSpeed = 400,
        damage = 10,
        projectileCount = 1,

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 166, y = 3, w = 12, h = 12 },
            scale = 2.5,
            animated = true,
            frameCount = 4,
            frameTime = 0.1,
            spacing = 5  -- Espace entre les frames
        }
    },

    enemy_spread = {
        name = "Enemy Spread",
        category = "enemy",

        fireRate = 2.0,
        projectileSpeed = 350,
        damage = 8,
        projectileCount = 3,
        spreadAngle = 45,

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 166, y = 3, w = 12, h = 12 },
            scale = 2.0,
            animated = true,
            frameCount = 4,
            frameTime = 0.1,
            spacing = 5
        }
    },

    enemy_aimed = {
        name = "Enemy Aimed",
        category = "enemy",

        fireRate = 2.5,
        projectileSpeed = 500,
        damage = 15,
        projectileCount = 1,

        -- Aims at player
        aimed = true,

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 166, y = 3, w = 12, h = 12 },  -- Même sprite
            scale = 2.5,
            animated = true,
            frameCount = 4,
            frameTime = 0.08,
            spacing = 5
        }
    },

    enemy_laser = {
        name = "Enemy Laser",
        category = "enemy",

        fireRate = 3.0,
        projectileSpeed = 800,
        damage = 25,
        projectileCount = 1,

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 1, y = 52, w = 32, h = 8 },
            scale = 2.0,
            animated = false
        }
    },

    -- ========================================================================
    -- BOSS WEAPONS
    -- ========================================================================

    boss_spread = {
        name = "Boss Spread",
        category = "boss",

        fireRate = 1.0,
        projectileSpeed = 300,
        damage = 15,
        projectileCount = 5,
        spreadAngle = 60,

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 166, y = 3, w = 12, h = 12 },
            scale = 3.0,
            animated = true,
            frameCount = 4,
            frameTime = 0.1,
            spacing = 5
        }
    },

    boss_laser_sweep = {
        name = "Boss Laser",
        category = "boss",

        fireRate = 0.1,  -- Rapid fire for sweep
        projectileSpeed = 1000,
        damage = 10,
        projectileCount = 1,

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 1, y = 52, w = 48, h = 12 },
            scale = 2.5,
            animated = false
        }
    },

    boss_bullet_hell = {
        name = "Boss Bullet Hell",
        category = "boss",

        fireRate = 0.05,
        projectileSpeed = 250,
        damage = 8,
        projectileCount = 1,

        -- Spiral pattern
        pattern = "spiral",
        spiralSpeed = 90,  -- Degrees per second

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 198, y = 3, w = 12, h = 12 },
            scale = 2.0,
            animated = true,
            frameCount = 4,
            frameTime = 0.08
        }
    },

    boss_homing = {
        name = "Boss Homing",
        category = "boss",

        fireRate = 1.5,
        projectileSpeed = 400,
        damage = 20,
        projectileCount = 2,

        homing = true,
        homingStrength = 3.0,
        homingRange = 800,
        homingDelay = 0.5,

        projectile = {
            texture = "enemies/enemy_bullets.png",
            normalRect = { x = 102, y = 18, w = 16, h = 16 },
            scale = 3.0,
            animated = true,
            frameCount = 2,
            frameTime = 0.15
        }
    }
}

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Get weapon config by name
function GetWeapon(weaponName)
    return WeaponsConfig[weaponName]
end

-- Get weapon stats at specific level
function GetWeaponAtLevel(weaponName, level)
    local weapon = WeaponsConfig[weaponName]
    if not weapon then return nil end

    level = math.max(1, math.min(level, 5))  -- Clamp 1-5

    local stats = weapon.levels and weapon.levels[level] or {}

    -- Merge base stats with level stats
    local result = {}
    for k, v in pairs(weapon) do
        if k ~= "levels" and k ~= "chargelevels" then
            result[k] = v
        end
    end
    for k, v in pairs(stats) do
        result[k] = v
    end

    return result
end

-- Get charge level data
function GetChargeLevel(weaponName, chargeTime)
    local weapon = WeaponsConfig[weaponName]
    if not weapon or not weapon.chargelevels then return nil end

    local result = nil
    for i, level in ipairs(weapon.chargelevels) do
        if chargeTime >= level.threshold then
            result = level
            result.level = i
        end
    end
    return result
end

print("[WeaponsConfig] Loaded " .. #WeaponsConfig .. " weapon types")
