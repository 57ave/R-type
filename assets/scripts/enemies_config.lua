-- ============================================================================
-- ENEMIES CONFIGURATION
-- All enemy types defined in Lua - fully data-driven
-- ============================================================================

EnemiesConfig = {
    -- ========================================================================
    -- BASIC ENEMIES
    -- ========================================================================

    -- Standard grunt enemy
    basic = {
        name = "Patapata",
        description = "Basic enemy that flies straight",
        category = "common",

    -- Stats (before difficulty scaling)
        health = 1,
        damage = 10,
        speed = 200,
        scoreValue = 100,

        -- Movement
        movement = {
            pattern = "straight",
            direction = "left"
        },

        -- Sprite configuration
        sprite = {
            texture = "enemies/r-typesheet5.png",
            frameWidth = 33,
            frameHeight = 36,
            scale = 2.5,
            startX = 0,
            startY = 0,
            spacing = 0
        },

        -- Animation
        animation = {
            frameCount = 8,
            frameTime = 0.1,
            loop = true
        },

        -- Hitbox
        hitbox = {
            width = 33,
            height = 32,
            offsetX = 0,
            offsetY = 0
        },

    -- Combat
    weapon = "enemy_spread",  -- Switch to spread to test different projectile visuals
    shootInterval = 1.5,

        -- On death
        deathEffect = "explosion_small",
        dropChance = 0.05,  -- 5% chance to drop power-up
        dropTable = { "speed_boost", "weapon_upgrade" }
    },

    -- Zigzag enemy
    zigzag = {
        name = "Ziggy",
        description = "Moves in a zigzag pattern",
        category = "common",

    health = 1,
        damage = 10,
        speed = 220,
        scoreValue = 150,

        movement = {
            pattern = "zigzag",
            direction = "left",
            amplitude = 80,
            frequency = 2.0
        },

        sprite = {
            texture = "enemies/r-typesheet3.png",
            frameWidth = 17,
            frameHeight = 18,
            scale = 2.5,
            startX = 0,
            startY = 0,
            spacing = 0
        },

        animation = {
            frameCount = 12,
            frameTime = 0.08,
            loop = true
        },

        hitbox = {
            width = 33,
            height = 32
        },

    weapon = "enemy_bullet",
    shootInterval = 1.2,

        deathEffect = "explosion_small",
        dropChance = 0.08,
        dropTable = { "speed_boost", "weapon_upgrade", "health_restore" }
    },

    -- Sine wave enemy
    sinewave = {
        name = "Weaver",
        description = "Smooth sine wave movement",
        category = "common",

    health = 100,
        damage = 12,
        speed = 180,
        scoreValue = 200,

        movement = {
            pattern = "sine_wave",
            direction = "left",
            amplitude = 120,
            frequency = 1.5
        },

        sprite = {
            texture = "enemies/r-typesheet3.png",
            frameWidth = 17,
            frameHeight = 18,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 12,
            frameTime = 0.08,
            loop = true
        },

        hitbox = {
            width = 34,
            height = 34
        },

    weapon = "enemy_aimed",
    shootInterval = 2.0,

        deathEffect = "explosion_small",
        dropChance = 0.10,
        dropTable = { "weapon_upgrade", "multi_shot" }
    },

    -- Kamikaze enemy
    kamikaze = {
        name = "Crasher",
        description = "Charges directly at player",
        category = "common",

    health = 1,
        damage = 25,
        speed = 400,
        scoreValue = 120,

        movement = {
            pattern = "chase",
            direction = "player",  -- Tracks player
            turnSpeed = 3.0
        },

        sprite = {
            texture = "enemies/r-typesheet8.png",
            frameWidth = 32,
            frameHeight = 32,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 4,
            frameTime = 0.05,
            loop = true
        },

        hitbox = {
            width = 28,
            height = 28
        },

    weapon = "enemy_spread",
    shootInterval = 1.0,

        deathEffect = "explosion_small",
        dropChance = 0.03,
        dropTable = { "speed_boost" }
    },

    -- ========================================================================
    -- MEDIUM ENEMIES
    -- ========================================================================

    -- Shooter enemy
    shooter = {
        name = "Gunner",
        description = "Shoots at player",
        category = "medium",

    health = 1,
        damage = 15,
        speed = 150,
        scoreValue = 300,

        movement = {
            pattern = "straight",
            direction = "left"
        },

        sprite = {
            texture = "enemies/r-typesheet12.png",
            frameWidth = 33,
            frameHeight = 30,
            scale = 2.5,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 2,
            frameTime = 0.15,
            loop = true
        },

        hitbox = {
            width = 33,
            height = 30
        },

        -- Combat
        weapon = "enemy_bullet",
        shootInterval = 2.0,
        shootPattern = "forward",

        deathEffect = "explosion_medium",
        dropChance = 0.15,
        dropTable = { "weapon_upgrade", "damage_boost", "multi_shot" }
    },

    -- Spread shooter
    spreader = {
        name = "Spreader",
        description = "Fires spread shots",
        category = "medium",

    health = 1,
        damage = 12,
        speed = 120,
        scoreValue = 400,

        movement = {
            pattern = "sinewave",
            direction = "left",
            amplitude = 60,
            frequency = 1.0
        },

        sprite = {
            texture = "enemies/r-typesheet14.png",
            frameWidth = 46,
            frameHeight = 44,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 3,
            frameTime = 0.12,
            loop = true
        },

        hitbox = {
            width = 42,
            height = 40
        },

        weapon = "enemy_spread",
        shootInterval = 2.5,
        shootPattern = "spread",

        deathEffect = "explosion_medium",
        dropChance = 0.18,
        dropTable = { "spread_shot", "weapon_upgrade" }
    },

    -- Armored enemy
    armored = {
        name = "Tank",
        description = "Heavily armored, slow",
        category = "medium",

    health = 1,
        damage = 20,
        speed = 80,
        scoreValue = 500,

        -- Defense properties
        armor = 5,              -- Reduces damage taken by flat amount
        armorPierceRequired = 10,  -- Damage below this does 1 damage

        movement = {
            pattern = "straight",
            direction = "left"
        },

        sprite = {
            texture = "enemies/r-typesheet17.png",
            frameWidth = 64,
            frameHeight = 48,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 2,
            frameTime = 0.2,
            loop = true
        },

        hitbox = {
            width = 60,
            height = 44
        },

        weapon = "enemy_aimed",
        shootInterval = 3.0,
        shootPattern = "aimed",

        deathEffect = "explosion_large",
        dropChance = 0.25,
        dropTable = { "shield", "weapon_upgrade", "extra_life" }
    },

    -- ========================================================================
    -- ELITE ENEMIES
    -- ========================================================================

    -- Turret (stationary)
    turret = {
        name = "Turret",
        description = "Stationary defense turret",
        category = "elite",

    health = 1,
        damage = 20,
        speed = 0,  -- Stationary
        scoreValue = 350,

        movement = {
            pattern = "stationary",
            attachToBackground = true  -- Scrolls with background
        },

        sprite = {
            texture = "enemies/r-typesheet21.png",
            frameWidth = 32,
            frameHeight = 32,
            scale = 2.5,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 4,
            frameTime = 0.2,
            loop = true
        },

        hitbox = {
            width = 28,
            height = 28
        },

        weapon = "enemy_aimed",
        shootInterval = 1.5,
        shootPattern = "aimed",
        trackPlayer = true,     -- Barrel follows player

        deathEffect = "explosion_medium",
        dropChance = 0.20,
        dropTable = { "damage_boost", "weapon_upgrade" }
    },

    -- Elite fighter
    elite_fighter = {
        name = "Ace",
        description = "Elite enemy with evasive maneuvers",
        category = "elite",

    health = 1,
        damage = 18,
        speed = 280,
        scoreValue = 600,

        movement = {
            pattern = "evasive",
            direction = "left",
            evadeDistance = 150,    -- Distance to start evading
            evadeSpeed = 400
        },

        sprite = {
            texture = "enemies/r-typesheet24.png",
            frameWidth = 48,
            frameHeight = 32,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 4,
            frameTime = 0.08,
            loop = true
        },

        hitbox = {
            width = 44,
            height = 28
        },

        weapon = "enemy_bullet",
        shootInterval = 1.2,
        shootPattern = "burst",
        burstCount = 3,
        burstInterval = 0.15,

        deathEffect = "explosion_medium",
        dropChance = 0.30,
        dropTable = { "laser_weapon", "homing_missile", "speed_boost" }
    },

    -- Formation leader
    formation_leader = {
        name = "Commander",
        description = "Leads enemy formations",
        category = "elite",

    health = 1,
        damage = 15,
        speed = 160,
        scoreValue = 800,

        -- Special: Spawns minions
        spawnsMinions = true,
        minionType = "basic",
        minionCount = 4,
        minionSpawnInterval = 3.0,
        maxMinions = 6,

        movement = {
            pattern = "hover",
            direction = "left",
            hoverX = 1500,      -- X position to hover at
            hoverRange = 100   -- Movement range while hovering
        },

        sprite = {
            texture = "enemies/r-typesheet26.png",
            frameWidth = 64,
            frameHeight = 64,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 4,
            frameTime = 0.15,
            loop = true
        },

        hitbox = {
            width = 56,
            height = 56
        },

        weapon = "enemy_spread",
        shootInterval = 2.0,
        shootPattern = "spread",

        deathEffect = "explosion_large",
        dropChance = 0.40,
        dropTable = { "force_pod", "extra_life", "bomb" }
    },

    -- ========================================================================
    -- SPECIAL ENEMIES
    -- ========================================================================

    -- Power-up carrier (always drops items)
    carrier = {
        name = "Cargo",
        description = "Carries power-ups",
        category = "special",

    health = 1,
        damage = 5,
        speed = 100,
        scoreValue = 50,

        movement = {
            pattern = "straight",
            direction = "left"
        },

        sprite = {
            texture = "enemies/r-typesheet28.png",
            frameWidth = 32,
            frameHeight = 32,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 4,
            frameTime = 0.15,
            loop = true
        },

        hitbox = {
            width = 30,
            height = 30
        },

    weapon = "enemy_bullet",
    shootInterval = 1.8,

        deathEffect = "explosion_small",
        dropChance = 1.0,  -- Always drops
        dropCount = 2,     -- Drops 2 items
        dropTable = { "weapon_upgrade", "speed_boost", "damage_boost", "health_restore" }
    },

    -- Shield enemy
    shielded = {
        name = "Barrier",
        description = "Protected by shield",
        category = "special",

    health = 1,
        damage = 15,
        speed = 140,
        scoreValue = 450,

        -- Shield properties
        hasShield = true,
        shieldHealth = 40,
        shieldRegen = 5.0,     -- HP per second when not taking damage
        shieldRegenDelay = 3.0,  -- Seconds before regen starts
        shieldDirection = "front",  -- "front", "all", "back"

        movement = {
            pattern = "zigzag",
            direction = "left",
            amplitude = 50,
            frequency = 1.5
        },

        sprite = {
            texture = "enemies/r-typesheet30.png",
            frameWidth = 48,
            frameHeight = 48,
            scale = 2.0,
            startX = 0,
            startY = 0
        },

        animation = {
            frameCount = 4,
            frameTime = 0.12,
            loop = true
        },

        hitbox = {
            width = 44,
            height = 44
        },

        weapon = "enemy_bullet",
        shootInterval = 2.0,

        deathEffect = "explosion_medium",
        dropChance = 0.20,
        dropTable = { "shield", "weapon_upgrade" }
    }
}

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Get enemy config by type
function GetEnemy(enemyType)
    return EnemiesConfig[enemyType]
end

-- Get scaled enemy stats
function GetEnemyScaled(enemyType)
    local enemy = EnemiesConfig[enemyType]
    if not enemy then return nil end

    local scaled = {}
    for k, v in pairs(enemy) do
        scaled[k] = v
    end

    -- Apply difficulty scaling
    scaled.health = ApplyDifficulty(enemy.health, "health")
    scaled.damage = ApplyDifficulty(enemy.damage, "damage")
    scaled.speed = ApplyDifficulty(enemy.speed, "speed")
    scaled.scoreValue = ApplyDifficulty(enemy.scoreValue, "score")

    if enemy.shootInterval then
        scaled.shootInterval = enemy.shootInterval / GetCurrentDifficulty().enemySpeedMult
    end

    return scaled
end

-- Get random enemy from category
function GetRandomEnemy(category)
    local enemies = {}
    for name, enemy in pairs(EnemiesConfig) do
        if enemy.category == category then
            table.insert(enemies, name)
        end
    end

    if #enemies > 0 then
        return enemies[math.random(#enemies)]
    end
    return nil
end

-- Get all enemy types
function GetAllEnemyTypes()
    local types = {}
    for name, _ in pairs(EnemiesConfig) do
        table.insert(types, name)
    end
    return types
end

print("[EnemiesConfig] Loaded enemy types")
