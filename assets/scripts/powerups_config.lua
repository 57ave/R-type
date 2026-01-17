-- ============================================================================
-- POWER-UPS AND ATTACHMENTS CONFIGURATION
-- All collectible items and player attachments
-- ============================================================================

PowerUpsConfig = {
    -- ========================================================================
    -- BASIC POWER-UPS (Collectible items)
    -- ========================================================================
    
    speed_boost = {
        name = "Speed Up",
        description = "Increases player speed",
        category = "boost",
        
        -- Effect
        effect = {
            type = "speed",
            value = 50,         -- Speed increase per pickup
            maxStacks = 5,      -- Maximum speed upgrades
            duration = 0,       -- 0 = permanent
        },
        
        -- Sprite
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 0, y = 0, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.15,
            loop = true
        },
        
        -- Pickup
        pickupSound = "powerup_collect",
        flashColor = { r = 0, g = 255, b = 255 },
        
        -- Spawn
        floatSpeed = 50,     -- Moves left slowly
        lifetime = 10.0      -- Disappears after 10 seconds
    },
    
    weapon_upgrade = {
        name = "Weapon Power",
        description = "Upgrades current weapon level",
        category = "weapon",
        
        effect = {
            type = "weapon_level",
            value = 1,
            maxStacks = 5
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 24, y = 0, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.12,
            loop = true
        },
        
        pickupSound = "powerup_collect",
        flashColor = { r = 255, g = 128, b = 0 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    damage_boost = {
        name = "Power Crystal",
        description = "Increases damage output",
        category = "boost",
        
        effect = {
            type = "damage",
            multiplier = 1.25,  -- 25% damage increase
            duration = 30.0    -- Temporary boost
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 48, y = 0, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.1,
            loop = true
        },
        
        pickupSound = "powerup_collect",
        flashColor = { r = 255, g = 0, b = 0 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    health_restore = {
        name = "Repair Kit",
        description = "Restores player health",
        category = "health",
        
        effect = {
            type = "health",
            value = 25,         -- HP restored
            canOverheal = false
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 72, y = 0, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.15,
            loop = true
        },
        
        pickupSound = "health_collect",
        flashColor = { r = 0, g = 255, b = 0 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    shield = {
        name = "Shield",
        description = "Temporary invincibility",
        category = "defense",
        
        effect = {
            type = "shield",
            duration = 10.0,
            hitPoints = 3,       -- Can take 3 hits
            reflectsBullets = false
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 96, y = 0, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.1,
            loop = true
        },
        
        pickupSound = "shield_collect",
        flashColor = { r = 100, g = 100, b = 255 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    extra_life = {
        name = "1UP",
        description = "Extra life",
        category = "special",
        
        effect = {
            type = "life",
            value = 1
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 120, y = 0, w = 24, h = 24 },
            scale = 2.5
        },
        
        animation = {
            frameCount = 2,
            frameTime = 0.3,
            loop = true
        },
        
        pickupSound = "oneup_collect",
        flashColor = { r = 255, g = 255, b = 0 },
        
        floatSpeed = 30,
        lifetime = 15.0
    },
    
    bomb = {
        name = "Bomb",
        description = "Screen-clearing bomb",
        category = "special",
        
        effect = {
            type = "bomb",
            value = 1,
            maxStacks = 3
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 144, y = 0, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.12,
            loop = true
        },
        
        pickupSound = "bomb_collect",
        flashColor = { r = 255, g = 0, b = 255 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    -- ========================================================================
    -- WEAPON CHANGE POWER-UPS
    -- ========================================================================
    
    laser_weapon = {
        name = "Laser Crystal",
        description = "Changes weapon to Laser",
        category = "weapon_change",
        
        effect = {
            type = "weapon_change",
            weapon = "laser",
            level = 1
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 0, y = 24, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.1,
            loop = true
        },
        
        pickupSound = "weapon_change",
        flashColor = { r = 0, g = 0, b = 255 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    spread_weapon = {
        name = "Spread Crystal",
        description = "Changes weapon to Spread",
        category = "weapon_change",
        
        effect = {
            type = "weapon_change",
            weapon = "spread_shot",
            level = 1
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 24, y = 24, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.1,
            loop = true
        },
        
        pickupSound = "weapon_change",
        flashColor = { r = 0, g = 255, b = 0 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    homing_weapon = {
        name = "Homing Crystal",
        description = "Changes weapon to Homing Missiles",
        category = "weapon_change",
        
        effect = {
            type = "weapon_change",
            weapon = "homing_missile",
            level = 1
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 48, y = 24, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.1,
            loop = true
        },
        
        pickupSound = "weapon_change",
        flashColor = { r = 255, g = 255, b = 0 },
        
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    wave_weapon = {
        name = "Wave Crystal",
        description = "Changes weapon to Wave Beam",
        category = "weapon_change",
        
        effect = {
            type = "weapon_change",
            weapon = "wave_beam",
            level = 1
        },
        
        sprite = {
            texture = "powerups/powerups.png",
            rect = { x = 72, y = 24, w = 24, h = 24 },
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.1,
            loop = true
        },
        
        pickupSound = "weapon_change",
        flashColor = { r = 128, g = 0, b = 255 },
        
        floatSpeed = 50,
        lifetime = 10.0
    }
}

-- ============================================================================
-- ATTACHMENTS CONFIGURATION (Force Pod, Options, etc.)
-- ============================================================================

AttachmentsConfig = {
    -- ========================================================================
    -- FORCE POD (Iconic R-Type attachment)
    -- ========================================================================
    force_pod = {
        name = "Force",
        description = "Indestructible attack pod",
        category = "force",
        
        -- States
        states = {
            detached = "floating",
            attached_front = "front",
            attached_back = "back",
            launching = "launch",
            returning = "return"
        },
        
        -- Levels (upgraded by collecting more Force power-ups)
        levels = {
            [1] = {
                name = "Force Lv1",
                contactDamage = 10,
                weaponType = "force_shot_1",
                size = 1.0
            },
            [2] = {
                name = "Force Lv2",
                contactDamage = 20,
                weaponType = "force_shot_2",
                size = 1.2
            },
            [3] = {
                name = "Force Lv3",
                contactDamage = 30,
                weaponType = "force_shot_3",
                size = 1.4
            }
        },
        
        -- Combat
        isInvincible = true,
        blocksEnemyBullets = true,
        contactDamage = 10,
        
        -- Attachment positions
        frontOffset = { x = 100, y = 0 },
        backOffset = { x = -60, y = 0 },
        
        -- Launch properties
        launchSpeed = 800,
        returnSpeed = 600,
        maxLaunchDistance = 600,
        
        -- Visual
        sprite = {
            texture = "attachments/force.png",
            frameWidth = 32,
            frameHeight = 32,
            scale = 2.0
        },
        
        animation = {
            frameCount = 8,
            frameTime = 0.08,
            loop = true
        },
        
        -- Sounds
        attachSound = "force_attach",
        launchSound = "force_launch",
        
        -- As power-up (collectible)
        asPowerUp = {
            floatSpeed = 40,
            lifetime = 15.0,
            pickupSound = "force_collect"
        }
    },
    
    -- ========================================================================
    -- OPTIONS (Trailing satellites like Gradius)
    -- ========================================================================
    option = {
        name = "Option",
        description = "Following attack satellite",
        category = "option",
        
        maxOptions = 4,  -- Maximum number of options
        
        -- Following behavior
        followDelay = 0.2,  -- Seconds of delay
        followDistance = 60,
        
        -- Combat
        mirrorsPlayerFire = true,
        independentFire = false,
        fireRate = 0.2,
        projectileType = "option_shot",
        damage = 8,
        
        -- Formations
        formations = {
            trail = {
                -- Options follow player path with delay
                delays = { 0.2, 0.4, 0.6, 0.8 }
            },
            spread = {
                -- Options spread out around player
                positions = {
                    { x = -50, y = -60 },
                    { x = -50, y = 60 },
                    { x = -100, y = -40 },
                    { x = -100, y = 40 }
                }
            },
            rotate = {
                -- Options rotate around player
                radius = 80,
                speed = 180,  -- Degrees per second
                offsets = { 0, 90, 180, 270 }
            },
            fixed = {
                -- Options stay at fixed positions
                positions = {
                    { x = 0, y = -60 },
                    { x = 0, y = 60 },
                    { x = -60, y = -30 },
                    { x = -60, y = 30 }
                }
            }
        },
        
        -- Visual
        sprite = {
            texture = "attachments/option.png",
            frameWidth = 24,
            frameHeight = 24,
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.1,
            loop = true
        },
        
        -- As power-up
        asPowerUp = {
            floatSpeed = 40,
            lifetime = 15.0,
            pickupSound = "option_collect"
        }
    },
    
    -- ========================================================================
    -- BIT (Defensive satellites)
    -- ========================================================================
    bit = {
        name = "Bit",
        description = "Defensive orb",
        category = "defense",
        
        maxBits = 2,
        
        -- Positioning
        positions = {
            { x = 40, y = -50 },
            { x = 40, y = 50 }
        },
        
        -- Combat
        blocksEnemyBullets = true,
        reflectsBullets = true,
        reflectDamage = 1.5,
        
        -- Movement
        orbitsPlayer = true,
        orbitSpeed = 120,
        orbitRadius = 70,
        
        -- Visual
        sprite = {
            texture = "attachments/bit.png",
            frameWidth = 16,
            frameHeight = 16,
            scale = 2.0
        },
        
        animation = {
            frameCount = 4,
            frameTime = 0.08,
            loop = true
        },
        
        asPowerUp = {
            floatSpeed = 40,
            lifetime = 15.0,
            pickupSound = "bit_collect"
        }
    }
}

-- ============================================================================
-- FORCE WEAPONS (What the Force Pod shoots based on level/type)
-- ============================================================================

ForceWeaponsConfig = {
    force_shot_1 = {
        name = "Force Shot Lv1",
        projectileCount = 1,
        damage = 8,
        speed = 800,
        fireRate = 0.2,
        
        -- Fires forward only
        directions = { 0 }
    },
    
    force_shot_2 = {
        name = "Force Shot Lv2",
        projectileCount = 2,
        damage = 10,
        speed = 800,
        fireRate = 0.18,
        
        -- Fires forward and diagonal
        directions = { 0, 30, -30 }
    },
    
    force_shot_3 = {
        name = "Force Shot Lv3",
        projectileCount = 3,
        damage = 12,
        speed = 800,
        fireRate = 0.15,
        
        -- Fires forward and back
        directions = { 0, 45, -45, 180 }
    },
    
    -- Laser type force
    force_laser = {
        name = "Force Laser",
        projectileCount = 2,
        damage = 15,
        speed = 1000,
        fireRate = 0.25,
        piercing = true,
        
        directions = { 0 }
    },
    
    -- Wave type force
    force_wave = {
        name = "Force Wave",
        projectileCount = 3,
        damage = 8,
        speed = 600,
        fireRate = 0.2,
        bouncing = true,
        maxBounces = 2,
        
        directions = { 0, 45, -45 }
    }
}

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Get power-up config
function GetPowerUp(powerUpType)
    return PowerUpsConfig[powerUpType]
end

-- Get attachment config
function GetAttachment(attachmentType)
    return AttachmentsConfig[attachmentType]
end

-- Get random power-up from drop table
function GetRandomDrop(dropTable, dropChance)
    if math.random() > dropChance then
        return nil
    end
    
    if dropTable and #dropTable > 0 then
        return dropTable[math.random(#dropTable)]
    end
    return nil
end

-- Apply power-up effect (returns modified player stats)
function ApplyPowerUpEffect(playerStats, powerUpType)
    local powerUp = PowerUpsConfig[powerUpType]
    if not powerUp then return playerStats end
    
    local effect = powerUp.effect
    local newStats = {}
    for k, v in pairs(playerStats) do
        newStats[k] = v
    end
    
    if effect.type == "speed" then
        newStats.speed = math.min(
            newStats.speed + effect.value,
            newStats.baseSpeed + (effect.value * effect.maxStacks)
        )
    elseif effect.type == "health" then
        if effect.canOverheal then
            newStats.health = newStats.health + effect.value
        else
            newStats.health = math.min(newStats.health + effect.value, newStats.maxHealth)
        end
    elseif effect.type == "weapon_level" then
        newStats.weaponLevel = math.min(newStats.weaponLevel + effect.value, effect.maxStacks)
    elseif effect.type == "life" then
        newStats.lives = newStats.lives + effect.value
    elseif effect.type == "bomb" then
        newStats.bombs = math.min(newStats.bombs + effect.value, effect.maxStacks)
    elseif effect.type == "weapon_change" then
        newStats.weapon = effect.weapon
        newStats.weaponLevel = effect.level
    end
    
    return newStats
end

print("[PowerUpsConfig] Loaded power-up and attachment configurations")
