-- ============================================================================
-- GAMEPLAY CONFIGURATION
-- All gameplay parameters are defined here - no hardcoded values in C++
-- This file is the master configuration for the entire game
-- ============================================================================

GameplayConfig = {
    -- ========================================================================
    -- PLAYER CONFIGURATION
    -- ========================================================================
    player = {
        -- Base stats
        baseHealth = 1,
        baseLives = 3,
        baseSpeed = 500,
        
        -- Invincibility after being hit
        invincibilityDuration = 2.0,
        
        -- Starting position
        startX = 150,
        startY = 540,
        
        -- Sprite configuration
        sprite = {
            texture = "players/r-typesheet1.png",
            frameWidth = 33,
            frameHeight = 17,
            scale = 3.0,
            defaultRow = 0,  -- Can be 0-4 for different ship colors
        },
        
        -- Animation states (column-based state machine)
        animation = {
            neutral = 2,      -- Center frame
            up = 4,           -- Tilted up
            down = 0,         -- Tilted down
            transitionSpeed = 0.15
        },
        
        -- Hitbox (relative to sprite)
        hitbox = {
            width = 33,
            height = 17,
            offsetX = 0,
            offsetY = 0
        },
        
        -- Starting weapon
        startingWeapon = "single_shot",
        startingWeaponLevel = 1
    },

    -- ========================================================================
    -- DIFFICULTY SETTINGS
    -- ========================================================================
    difficulty = {
        easy = {
            name = "Easy",
            enemyHealthMult = 0.7,
            enemyDamageMult = 0.5,
            enemySpeedMult = 0.8,
            spawnRateMult = 1.5,  -- Higher = slower spawns
            scoreMultiplier = 0.5,
            playerLives = 5,
            continueCount = 3
        },
        normal = {
            name = "Normal",
            enemyHealthMult = 1.0,
            enemyDamageMult = 1.0,
            enemySpeedMult = 1.0,
            spawnRateMult = 1.0,
            scoreMultiplier = 1.0,
            playerLives = 3,
            continueCount = 2
        },
        hard = {
            name = "Hard",
            enemyHealthMult = 1.5,
            enemyDamageMult = 1.5,
            enemySpeedMult = 1.2,
            spawnRateMult = 0.8,
            scoreMultiplier = 1.5,
            playerLives = 3,
            continueCount = 1
        },
        insane = {
            name = "Insane",
            enemyHealthMult = 2.0,
            enemyDamageMult = 2.0,
            enemySpeedMult = 1.5,
            spawnRateMult = 0.5,
            scoreMultiplier = 2.0,
            playerLives = 1,
            continueCount = 0
        }
    },
    
    -- Current difficulty (can be changed from menu)
    currentDifficulty = "normal",

    -- ========================================================================
    -- SCREEN / GAMEPLAY AREA
    -- ========================================================================
    screen = {
        width = 1920,
        height = 1080,
        
        -- Play area bounds (where player can move)
        playArea = {
            left = 50,
            right = 1870,
            top = 50,
            bottom = 1030
        },
        
        -- Enemy spawn zone
        spawnZone = {
            x = 1950,  -- Off-screen right
            minY = 100,
            maxY = 980
        },
        
        -- Background scrolling
        backgroundSpeed = 200
    },

    -- ========================================================================
    -- SCORING SYSTEM
    -- ========================================================================
    scoring = {
        -- Base points per enemy type (defined in enemy configs)
        killMultiplier = 1.0,
        
        -- Combo system
        combo = {
            enabled = true,
            timeWindow = 2.0,       -- Seconds to keep combo alive
            maxMultiplier = 10.0,   -- Maximum combo multiplier
            incrementPerKill = 0.5  -- Multiplier increase per kill
        },
        
        -- Bonuses
        bonuses = {
            waveComplete = 500,
            stageComplete = 5000,
            noDeathBonus = 10000,
            bossKill = 10000,
            perfectBoss = 5000,  -- Kill boss without taking damage
            speedBonus = 3000,  -- Complete stage under time limit
            allEnemiesKilled = 2000
        },
        
        -- Extra life threshold
        extraLifeScore = 50000
    },

    -- ========================================================================
    -- GLOBAL COMBAT SETTINGS
    -- ========================================================================
    combat = {
        -- Friendly fire
        friendlyFire = false,
        
        -- Collision layers
        layers = {
            player = 1,
            playerBullet = 2,
            enemy = 4,
            enemyBullet = 8,
            powerup = 16,
            boss = 32,
            bossWeakpoint = 64
        },
        
        -- What collides with what
        collisionMatrix = {
            player = { "enemy", "enemyBullet", "powerup", "boss" },
            playerBullet = { "enemy", "boss", "bossWeakpoint" },
            enemy = { "player", "playerBullet" },
            enemyBullet = { "player" }
        }
    }
}

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Get current difficulty settings
function GetCurrentDifficulty()
    return GameplayConfig.difficulty[GameplayConfig.currentDifficulty]
end

-- Apply difficulty scaling to a value
function ApplyDifficulty(baseValue, statType)
    local diff = GetCurrentDifficulty()
    
    if statType == "health" then
        return math.floor(baseValue * diff.enemyHealthMult)
    elseif statType == "damage" then
        return math.floor(baseValue * diff.enemyDamageMult)
    elseif statType == "speed" then
        return baseValue * diff.enemySpeedMult
    elseif statType == "spawnRate" then
        return baseValue * diff.spawnRateMult
    elseif statType == "score" then
        return math.floor(baseValue * diff.scoreMultiplier)
    end
    
    return baseValue
end

-- Get random Y position in spawn zone
function GetRandomSpawnY()
    local zone = GameplayConfig.screen.spawnZone
    return zone.minY + math.random() * (zone.maxY - zone.minY)
end

-- Check if position is in play area
function IsInPlayArea(x, y)
    local area = GameplayConfig.screen.playArea
    return x >= area.left and x <= area.right and y >= area.top and y <= area.bottom
end

print("[GameplayConfig] Loaded - Difficulty: " .. GameplayConfig.currentDifficulty)
