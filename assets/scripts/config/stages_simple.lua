-- ============================================================================
-- SIMPLIFIED STAGES CONFIGURATION
-- 2 levels with time-based waves + boss at the end
-- Works for both solo and multiplayer
-- ============================================================================

SimpleStagesConfig = {
    -- ========================================================================
    -- STAGE 1 - SPACE COLONY (60 seconds + boss)
    -- ========================================================================
    stage1 = {
        name = "Space Colony",
        description = "The Bydo have attacked the orbital station",
        stageNumber = 1,
        
        -- Stage duration (time before boss appears)
        duration = 60.0,
        
        -- Background
        background = {
            texture = "background.png",
            scrollSpeed = 200
        },
        
        -- Music
        music = "stage1_bgm",
        bossMusic = "boss_bgm",
        
        -- Boss for this stage
        boss = "stage1_boss",
        
        -- Waves (time-based, automatic progression)
        waves = {
            -- Wave 1: Easy start (0-30 seconds)
            {
                name = "First Contact",
                startTime = 0.0,
                endTime = 30.0,
                
                -- Spawn configuration
                spawnInterval = 2.0,  -- Spawn enemies every 2 seconds
                enemyTypes = { "basic", "zigzag" },
                enemyWeights = { 70, 30 },  -- % chance for each type
                
                -- Y positions for spawns
                spawnYMin = 100,
                spawnYMax = 900,
                
                -- Power-up drop at end of wave
                reward = { type = "speed_boost", time = 28.0 }
            },
            
            -- Wave 2: Harder (30-60 seconds)
            {
                name = "Pressure",
                startTime = 30.0,
                endTime = 60.0,
                
                spawnInterval = 1.5,  -- Faster spawns
                enemyTypes = { "basic", "zigzag", "shooter", "kamikaze" },
                enemyWeights = { 40, 25, 25, 10 },
                
                spawnYMin = 100,
                spawnYMax = 900,
                
                reward = { type = "shield", time = 55.0 }
            }
        },
        
        -- Multiplayer scaling
        multiplayerScaling = {
            enemyHealthMultiplier = 0.25,   -- +25% HP per extra player
            spawnRateMultiplier = 0.15,     -- +15% spawn rate per extra player
            bossHealthMultiplier = 0.5      -- +50% boss HP per extra player
        },
        
        -- Stage completion rewards
        completionBonus = 5000,
        perfectBonus = 10000
    },
    
    -- ========================================================================
    -- STAGE 2 - ASTEROID BELT (90 seconds + boss)
    -- ========================================================================
    stage2 = {
        name = "Asteroid Belt",
        description = "Navigate through the deadly asteroid field",
        stageNumber = 2,
        
        duration = 90.0,
        
        background = {
            texture = "backgrounds/stage2_bg.png",
            scrollSpeed = 250
        },
        
        music = "stage2_bgm",
        bossMusic = "boss_bgm",
        
        boss = "stage2_boss",
        
        waves = {
            -- Wave 1: Medium difficulty (0-45 seconds)
            {
                name = "Asteroid Field",
                startTime = 0.0,
                endTime = 45.0,
                
                spawnInterval = 1.8,
                enemyTypes = { "basic", "zigzag", "sinewave", "shooter" },
                enemyWeights = { 30, 25, 25, 20 },
                
                spawnYMin = 100,
                spawnYMax = 900,
                
                reward = { type = "weapon_upgrade", time = 40.0 }
            },
            
            -- Wave 2: Hard (45-90 seconds)
            {
                name = "Mining Facility",
                startTime = 45.0,
                endTime = 90.0,
                
                spawnInterval = 1.2,
                enemyTypes = { "zigzag", "shooter", "kamikaze", "armored" },
                enemyWeights = { 25, 30, 25, 20 },
                
                spawnYMin = 100,
                spawnYMax = 900,
                
                reward = { type = "shield", time = 85.0 }
            }
        },
        
        multiplayerScaling = {
            enemyHealthMultiplier = 0.3,
            spawnRateMultiplier = 0.2,
            bossHealthMultiplier = 0.6
        },
        
        completionBonus = 7500,
        perfectBonus = 15000
    }
}

-- ============================================================================
-- SIMPLIFIED BOSS CONFIGURATIONS
-- Phases with different attacks and minion spawns
-- ============================================================================

SimpleBossConfig = {
    -- Stage 1 Boss
    stage1_boss = {
        name = "Dobkeratops",
        
        -- Base stats (scaled in multiplayer)
        health = 300,
        scoreValue = 10000,
        
        -- Entry animation
        entry = {
            startX = 2200,
            targetX = 1450,
            duration = 3.0
        },
        
        -- Sprite
        sprite = {
            texture = "enemies/r-typesheet44.png",
            frameWidth = 160,
            frameHeight = 128,
            scale = 2.0
        },
        
        -- Movement
        movement = {
            pattern = "hover",
            amplitude = 80,
            frequency = 0.5
        },
        
        -- 3 Phases
        phases = {
            -- Phase 1: 100% - 66% HP
            {
                name = "Normal",
                healthThreshold = 1.0,
                attacks = { "spread_shot", "aimed_shot" },
                attackCooldown = 2.5,
                spawnMinions = false
            },
            -- Phase 2: 66% - 33% HP
            {
                name = "Aggressive",
                healthThreshold = 0.66,
                attacks = { "spread_shot", "laser_sweep", "aimed_shot" },
                attackCooldown = 2.0,
                spawnMinions = true,
                minionType = "basic",
                minionCount = 2,
                minionInterval = 8.0
            },
            -- Phase 3: 33% - 0% HP (Rage)
            {
                name = "Rage",
                healthThreshold = 0.33,
                attacks = { "bullet_hell", "laser_sweep", "charge_attack" },
                attackCooldown = 1.5,
                speedMultiplier = 1.5,
                spawnMinions = true,
                minionType = "kamikaze",
                minionCount = 3,
                minionInterval = 6.0
            }
        },
        
        -- Attack definitions
        attacks = {
            spread_shot = {
                type = "projectile",
                projectileCount = 5,
                spreadAngle = 60,
                damage = 10
            },
            aimed_shot = {
                type = "projectile",
                projectileCount = 3,
                aimedAtPlayer = true,
                damage = 15
            },
            laser_sweep = {
                type = "sweep",
                duration = 2.0,
                sweepAngle = 90,
                damage = 20
            },
            bullet_hell = {
                type = "pattern",
                pattern = "spiral",
                duration = 3.0,
                damage = 10
            },
            charge_attack = {
                type = "movement",
                chargeSpeed = 600,
                damage = 50
            }
        }
    },
    
    -- Stage 2 Boss
    stage2_boss = {
        name = "Gomander",
        
        health = 500,
        scoreValue = 15000,
        
        entry = {
            startX = 2200,
            targetX = 1400,
            duration = 4.0
        },
        
        sprite = {
            texture = "enemies/boss2.png",
            frameWidth = 200,
            frameHeight = 160,
            scale = 2.0
        },
        
        movement = {
            pattern = "hover",
            amplitude = 100,
            frequency = 0.4
        },
        
        phases = {
            {
                name = "Normal",
                healthThreshold = 1.0,
                attacks = { "triple_shot", "missile_barrage" },
                attackCooldown = 2.0,
                spawnMinions = false
            },
            {
                name = "Shield Phase",
                healthThreshold = 0.66,
                attacks = { "triple_shot", "laser_grid", "missile_barrage" },
                attackCooldown = 1.8,
                hasShield = true,
                shieldHealth = 50,
                spawnMinions = true,
                minionType = "shooter",
                minionCount = 2,
                minionInterval = 10.0
            },
            {
                name = "Final Form",
                healthThreshold = 0.33,
                attacks = { "mega_laser", "bullet_storm", "summon_turrets" },
                attackCooldown = 1.2,
                speedMultiplier = 1.3,
                spawnMinions = true,
                minionType = "armored",
                minionCount = 2,
                minionInterval = 8.0
            }
        },
        
        attacks = {
            triple_shot = {
                type = "projectile",
                projectileCount = 3,
                spreadAngle = 30,
                damage = 15
            },
            missile_barrage = {
                type = "homing",
                missileCount = 4,
                damage = 20
            },
            laser_grid = {
                type = "pattern",
                pattern = "grid",
                duration = 2.5,
                damage = 25
            },
            mega_laser = {
                type = "beam",
                duration = 3.0,
                damage = 40
            },
            bullet_storm = {
                type = "pattern",
                pattern = "random",
                duration = 4.0,
                damage = 10
            },
            summon_turrets = {
                type = "spawn",
                spawnType = "turret",
                spawnCount = 2
            }
        }
    }
}

-- ============================================================================
-- SIMPLIFIED POWER-UPS (Shield and Speed Boost)
-- ============================================================================

SimplePowerUpsConfig = {
    shield = {
        name = "Shield",
        description = "Temporary invincibility shield",
        
        effect = {
            type = "shield",
            duration = 8.0,       -- 8 seconds of shield
            hitPoints = 3,        -- Can absorb 3 hits
            cooldownAfter = 30.0  -- 30 seconds before can get another
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
        
        -- Visual effect on player when active
        playerEffect = {
            color = { r = 100, g = 150, b = 255, a = 150 },
            pulseSpeed = 2.0
        },
        
        pickupSound = "shield_collect",
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    speed_boost = {
        name = "Speed Up",
        description = "Temporary speed boost",
        
        effect = {
            type = "speed",
            multiplier = 1.5,     -- 50% faster
            duration = 10.0,      -- 10 seconds
            stackable = false     -- Can't stack multiple speed boosts
        },
        
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
        
        playerEffect = {
            trailEffect = true,
            trailColor = { r = 0, g = 255, b = 255, a = 100 }
        },
        
        pickupSound = "powerup_collect",
        floatSpeed = 50,
        lifetime = 10.0
    },
    
    weapon_upgrade = {
        name = "Weapon Power",
        description = "Permanently upgrades weapon level",
        
        effect = {
            type = "weapon_level",
            value = 1,
            maxLevel = 5,
            permanent = true
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
        floatSpeed = 50,
        lifetime = 10.0
    }
}

-- ============================================================================
-- API FUNCTIONS
-- ============================================================================

-- Get stage config
function GetSimpleStage(stageNumber)
    local stageKey = "stage" .. stageNumber
    return SimpleStagesConfig[stageKey]
end

-- Get boss config
function GetSimpleBoss(bossId)
    return SimpleBossConfig[bossId]
end

-- Get power-up config
function GetSimplePowerUp(powerUpType)
    return SimplePowerUpsConfig[powerUpType]
end

-- Get current wave based on time
function GetCurrentWave(stageNumber, currentTime)
    local stage = GetSimpleStage(stageNumber)
    if not stage then return nil end
    
    for i, wave in ipairs(stage.waves) do
        if currentTime >= wave.startTime and currentTime < wave.endTime then
            return wave, i
        end
    end
    
    -- Time exceeded all waves = boss time
    return nil, -1  -- -1 indicates boss phase
end

-- Check if it's boss time
function IsBossTime(stageNumber, currentTime)
    local stage = GetSimpleStage(stageNumber)
    if not stage then return false end
    return currentTime >= stage.duration
end

-- Get boss phase based on health percentage
function GetBossPhase(bossId, healthPercent)
    local boss = GetSimpleBoss(bossId)
    if not boss then return nil end
    
    for i = #boss.phases, 1, -1 do
        if healthPercent <= boss.phases[i].healthThreshold then
            return boss.phases[i], i
        end
    end
    
    return boss.phases[1], 1
end

-- Scale stats for multiplayer
function ScaleForMultiplayer(stageNumber, playerCount)
    local stage = GetSimpleStage(stageNumber)
    if not stage or playerCount <= 1 then
        return { enemyHealth = 1.0, spawnRate = 1.0, bossHealth = 1.0 }
    end
    
    local extraPlayers = playerCount - 1
    local scaling = stage.multiplayerScaling
    
    return {
        enemyHealth = 1.0 + (scaling.enemyHealthMultiplier * extraPlayers),
        spawnRate = 1.0 + (scaling.spawnRateMultiplier * extraPlayers),
        bossHealth = 1.0 + (scaling.bossHealthMultiplier * extraPlayers)
    }
end

-- Pick random enemy from wave
function GetRandomEnemyFromWave(wave)
    if not wave or not wave.enemyTypes then return "basic" end
    
    local totalWeight = 0
    for _, weight in ipairs(wave.enemyWeights) do
        totalWeight = totalWeight + weight
    end
    
    local roll = math.random(1, totalWeight)
    local cumulative = 0
    
    for i, weight in ipairs(wave.enemyWeights) do
        cumulative = cumulative + weight
        if roll <= cumulative then
            return wave.enemyTypes[i]
        end
    end
    
    return wave.enemyTypes[1]
end

-- Get random Y position for spawn
function GetRandomSpawnY(wave)
    if not wave then return 400 end
    return math.random(wave.spawnYMin, wave.spawnYMax)
end

print("[Config] Simple stages configuration loaded")
print("[Config] Stages: 2, Power-ups: shield, speed_boost, weapon_upgrade")
