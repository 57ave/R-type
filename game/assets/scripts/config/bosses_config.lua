-- ============================================================================
-- BOSSES CONFIGURATION
-- All boss types with phases, attack patterns, and weak points
-- ============================================================================

BossesConfig = {
    -- ========================================================================
    -- STAGE 1 BOSS - DOBKERATOPS
    -- ========================================================================
    stage1_boss = {
        name = "Dobkeratops",
        description = "Giant alien life form",
        
        -- Base stats
        health = 500,
        maxHealth = 500,
        scoreValue = 10000,
        
        -- Entry animation
        entry = {
            startX = 2200,
            startY = 540,
            targetX = 1450,
            targetY = 540,
            duration = 4.0,
            sound = "boss_entrance"
        },
        
        -- Sprite configuration
        sprite = {
            texture = "enemies/r-typesheet44.png",
            frameWidth = 160,
            frameHeight = 128,
            scale = 2.0,
            startX = 0,
            startY = 0
        },
        
        animation = {
            frameCount = 2,
            frameTime = 0.3,
            loop = true
        },
        
        hitbox = {
            width = 140,
            height = 110
        },
        
        -- Movement while active
        movement = {
            pattern = "hover",
            amplitude = 80,
            frequency = 0.5,
            horizontalRange = 100
        },
        
        -- Phase system
        phases = {
            {
                name = "Phase 1",
                healthThreshold = 1.0,  -- 100% to 66%
                attacks = { "spread_shot", "aimed_shot" },
                attackCooldown = 2.0,
                speedMultiplier = 1.0,
                
                -- Special phase properties
                shieldActive = false
            },
            {
                name = "Phase 2",
                healthThreshold = 0.66,  -- 66% to 33%
                attacks = { "spread_shot", "laser_sweep", "spawn_minions" },
                attackCooldown = 1.5,
                speedMultiplier = 1.2,
                
                -- Phase 2 activates shield
                shieldActive = true,
                shieldHealth = 50
            },
            {
                name = "Rage Mode",
                healthThreshold = 0.33,  -- 33% to 0%
                attacks = { "bullet_hell", "laser_sweep", "charge_attack" },
                attackCooldown = 1.0,
                speedMultiplier = 1.5,
                
                -- Visual change
                spriteRow = 1,  -- Different sprite row for damaged look
                colorTint = { r = 255, g = 100, b = 100 }
            }
        },
        
        -- Attack patterns
        attacks = {
            spread_shot = {
                type = "projectile",
                weapon = "boss_spread",
                duration = 0.5,
                projectileCount = 5,
                spreadAngle = 60,
                firePoint = { x = -50, y = 0 }
            },
            aimed_shot = {
                type = "projectile",
                weapon = "enemy_aimed",
                duration = 0.3,
                projectileCount = 3,
                burstInterval = 0.2,
                firePoint = { x = -50, y = 0 }
            },
            laser_sweep = {
                type = "sweep",
                weapon = "boss_laser_sweep",
                duration = 3.0,
                sweepAngle = 90,  -- Degrees to sweep
                sweepSpeed = 30,  -- Degrees per second
                firePoint = { x = -60, y = 30 }
            },
            bullet_hell = {
                type = "pattern",
                weapon = "boss_bullet_hell",
                duration = 4.0,
                pattern = "spiral",
                spiralArms = 4,
                spiralSpeed = 60,
                firePoint = { x = -40, y = 0 }
            },
            spawn_minions = {
                type = "spawn",
                enemyType = "basic",
                spawnCount = 4,
                spawnInterval = 0.5,
                spawnPositions = {
                    { x = 100, y = -80 },
                    { x = 100, y = 80 },
                    { x = 50, y = -40 },
                    { x = 50, y = 40 }
                }
            },
            charge_attack = {
                type = "movement",
                duration = 2.0,
                chargeSpeed = 600,
                chargeDirection = "player",
                returnAfter = true,
                damage = 50
            }
        },
        
        -- Weak point (tail in original R-Type)
        weakPoint = {
            enabled = true,
            name = "Core",
            offsetX = 0,
            offsetY = 20,
            width = 40,
            height = 40,
            damageMultiplier = 2.0,
            
            -- Visual
            sprite = {
                texture = "enemies/r-typesheet44.png",
                frameWidth = 32,
                frameHeight = 32,
                startX = 320,
                startY = 0
            },
            animation = {
                frameCount = 4,
                frameTime = 0.1,
                loop = true
            }
        },
        
        -- Death sequence
        death = {
            duration = 3.0,
            explosionCount = 10,
            explosionInterval = 0.3,
            sound = "boss_death",
            screenShake = true,
            screenShakeDuration = 2.0,
            dropPowerUp = "force_pod"
        },
        
        -- UI
        showHealthBar = true,
        healthBarWidth = 400,
        healthBarPosition = { x = 760, y = 50 }
    },
    
    -- ========================================================================
    -- STAGE 2 BOSS - GOMANDER
    -- ========================================================================
    stage2_boss = {
        name = "Gomander",
        description = "Giant snake-like warship",
        
        health = 800,
        maxHealth = 800,
        scoreValue = 15000,
        
        entry = {
            startX = 2400,
            startY = 300,
            targetX = 1400,
            targetY = 540,
            duration = 5.0,
            sound = "boss_entrance"
        },
        
        sprite = {
            texture = "enemies/r-typesheet38.png",
            frameWidth = 128,
            frameHeight = 96,
            scale = 2.5,
            startX = 0,
            startY = 0
        },
        
        animation = {
            frameCount = 2,
            frameTime = 0.25,
            loop = true
        },
        
        hitbox = {
            width = 120,
            height = 90
        },
        
        movement = {
            pattern = "snake",
            amplitude = 150,
            frequency = 0.3,
            segments = 5  -- Multi-part boss
        },
        
        phases = {
            {
                name = "Phase 1",
                healthThreshold = 1.0,
                attacks = { "tail_whip", "spread_shot" },
                attackCooldown = 2.5,
                speedMultiplier = 1.0
            },
            {
                name = "Phase 2",
                healthThreshold = 0.60,
                attacks = { "tail_whip", "laser_beam", "spawn_parasites" },
                attackCooldown = 2.0,
                speedMultiplier = 1.2
            },
            {
                name = "Phase 3",
                healthThreshold = 0.30,
                attacks = { "coil_attack", "laser_beam", "bullet_spray" },
                attackCooldown = 1.5,
                speedMultiplier = 1.4
            }
        },
        
        attacks = {
            spread_shot = {
                type = "projectile",
                weapon = "boss_spread",
                duration = 0.5,
                projectileCount = 7,
                spreadAngle = 80,
                firePoint = { x = -60, y = 0 }
            },
            tail_whip = {
                type = "melee",
                damage = 30,
                duration = 1.5,
                hitboxWidth = 200,
                hitboxHeight = 100
            },
            laser_beam = {
                type = "beam",
                weapon = "boss_laser_sweep",
                duration = 2.0,
                beamWidth = 40,
                damage = 5,  -- Per frame
                firePoint = { x = -80, y = 0 }
            },
            spawn_parasites = {
                type = "spawn",
                enemyType = "kamikaze",
                spawnCount = 6,
                spawnInterval = 0.3
            },
            coil_attack = {
                type = "special",
                duration = 3.0,
                pattern = "coil",
                damage = 40
            },
            bullet_spray = {
                type = "projectile",
                weapon = "enemy_bullet",
                duration = 2.0,
                projectileCount = 20,
                sprayAngle = 360,
                fireInterval = 0.1
            }
        },
        
        -- Multiple parts (segments)
        parts = {
            {
                name = "Head",
                isMainPart = true,
                offsetX = 0,
                offsetY = 0
            },
            {
                name = "Segment 1",
                offsetX = 100,
                offsetY = 0,
                health = 100,
                canBeDestroyed = true,
                disablesAttack = "tail_whip"
            },
            {
                name = "Segment 2",
                offsetX = 200,
                offsetY = 0,
                health = 100,
                canBeDestroyed = true
            },
            {
                name = "Tail",
                offsetX = 300,
                offsetY = 0,
                health = 150,
                canBeDestroyed = true,
                isWeakPoint = true,
                damageMultiplier = 1.5
            }
        },
        
        death = {
            duration = 4.0,
            explosionCount = 15,
            explosionInterval = 0.25,
            sound = "boss_death",
            screenShake = true,
            screenShakeDuration = 3.0,
            dropPowerUp = "extra_life"
        },
        
        showHealthBar = true,
        healthBarWidth = 500
    },
    
    -- ========================================================================
    -- STAGE 3 BOSS - BATTLESHIP
    -- ========================================================================
    stage3_boss = {
        name = "Battleship Green",
        description = "Massive warship with multiple turrets",
        
        health = 1200,
        maxHealth = 1200,
        scoreValue = 20000,
        
        entry = {
            startX = 2600,
            startY = 540,
            targetX = 1300,
            targetY = 540,
            duration = 6.0,
            sound = "boss_entrance"
        },
        
        sprite = {
            texture = "enemies/r-typesheet40.png",
            frameWidth = 256,
            frameHeight = 160,
            scale = 1.5,
            startX = 0,
            startY = 0
        },
        
        animation = {
            frameCount = 1,
            frameTime = 1.0,
            loop = false
        },
        
        hitbox = {
            width = 240,
            height = 150
        },
        
        movement = {
            pattern = "slow_advance",
            speed = 30,
            minX = 1100  -- Doesn't go further left than this
        },
        
        phases = {
            {
                name = "All Turrets",
                healthThreshold = 1.0,
                attacks = { "turret_barrage", "missile_salvo" },
                attackCooldown = 1.0,
                speedMultiplier = 1.0
            },
            {
                name = "Heavy Fire",
                healthThreshold = 0.50,
                attacks = { "main_cannon", "turret_barrage", "missile_salvo" },
                attackCooldown = 0.8,
                speedMultiplier = 1.0
            },
            {
                name = "Desperation",
                healthThreshold = 0.25,
                attacks = { "main_cannon", "bullet_wall", "ram_attack" },
                attackCooldown = 0.5,
                speedMultiplier = 1.5
            }
        },
        
        attacks = {
            turret_barrage = {
                type = "multi_fire",
                duration = 2.0,
                firePoints = { "turret_top", "turret_mid", "turret_bottom" },
                weapon = "enemy_aimed",
                fireInterval = 0.3
            },
            missile_salvo = {
                type = "projectile",
                weapon = "boss_homing",
                projectileCount = 4,
                fireInterval = 0.5,
                firePoint = { x = -100, y = -20 }
            },
            main_cannon = {
                type = "beam",
                duration = 3.0,
                chargeTime = 1.5,
                beamWidth = 60,
                damage = 10,
                firePoint = { x = -120, y = 0 }
            },
            bullet_wall = {
                type = "pattern",
                duration = 3.0,
                pattern = "wall",
                wallHeight = 800,
                gapSize = 150,
                gapPositions = 2,
                scrollSpeed = 400
            },
            ram_attack = {
                type = "movement",
                duration = 3.0,
                chargeSpeed = 400,
                chargeDirection = "left",
                damage = 100
            }
        },
        
        -- Destroyable turrets
        parts = {
            {
                name = "turret_top",
                offsetX = -50,
                offsetY = -60,
                health = 100,
                canBeDestroyed = true,
                weapon = "enemy_aimed",
                fireRate = 1.5,
                rotation = "track_player"
            },
            {
                name = "turret_mid",
                offsetX = -80,
                offsetY = 0,
                health = 100,
                canBeDestroyed = true,
                weapon = "enemy_spread",
                fireRate = 2.0,
                rotation = "track_player"
            },
            {
                name = "turret_bottom",
                offsetX = -50,
                offsetY = 60,
                health = 100,
                canBeDestroyed = true,
                weapon = "enemy_aimed",
                fireRate = 1.5,
                rotation = "track_player"
            },
            {
                name = "bridge",
                offsetX = 30,
                offsetY = -40,
                health = 200,
                canBeDestroyed = true,
                isWeakPoint = true,
                damageMultiplier = 2.0,
                criticalPart = true  -- Destroying this ends boss
            }
        },
        
        death = {
            duration = 5.0,
            explosionCount = 25,
            explosionInterval = 0.2,
            chainExplosion = true,
            sound = "boss_death",
            screenShake = true,
            screenShakeDuration = 4.0,
            dropPowerUp = "option"
        },
        
        showHealthBar = true,
        healthBarWidth = 600
    }
}

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Get boss config
function GetBoss(bossType)
    return BossesConfig[bossType]
end

-- Get boss for specific stage
function GetBossForStage(stageNumber)
    local bossName = "stage" .. stageNumber .. "_boss"
    return BossesConfig[bossName]
end

-- Get current phase for boss
function GetBossPhase(bossType, currentHealthPercent)
    local boss = BossesConfig[bossType]
    if not boss then return nil end
    
    for i = #boss.phases, 1, -1 do
        if currentHealthPercent <= boss.phases[i].healthThreshold then
            return boss.phases[i], i
        end
    end
    
    return boss.phases[1], 1
end

-- Get attack config
function GetBossAttack(bossType, attackName)
    local boss = BossesConfig[bossType]
    if not boss or not boss.attacks then return nil end
    return boss.attacks[attackName]
end

-- Scale boss for difficulty
function GetBossScaled(bossType)
    local boss = BossesConfig[bossType]
    if not boss then return nil end
    
    local scaled = {}
    for k, v in pairs(boss) do
        scaled[k] = v
    end
    
    scaled.health = ApplyDifficulty(boss.health, "health")
    scaled.maxHealth = scaled.health
    
    return scaled
end

print("[BossesConfig] Loaded boss configurations")
