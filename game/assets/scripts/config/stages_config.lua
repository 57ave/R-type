-- ============================================================================
-- STAGES AND WAVES CONFIGURATION
-- Complete level/wave definitions - data-driven level design
-- ============================================================================

StagesConfig = {
    -- ========================================================================
    -- STAGE 1 - SPACE COLONY
    -- ========================================================================
    stage1 = {
        name = "Space Colony",
        description = "The Bydo have attacked the orbital station",
        stageNumber = 1,
        
        -- Background
        background = {
            texture = "background.png",
            scrollSpeed = 200
        },
        
        -- Music
        music = "stage1_bgm",
        bossMusic = "boss_bgm",
        
        -- Stage settings
        duration = 180.0,
        
        -- Waves
        waves = {
            -- Wave 1: Introduction
            {
                name = "First Contact",
                startTime = 0.0,
                duration = 25.0,
                
                spawns = {
                    { time = 1.0,  enemy = "basic", y = 200, pattern = "straight" },
                    { time = 1.5,  enemy = "basic", y = 400, pattern = "straight" },
                    { time = 2.0,  enemy = "basic", y = 600, pattern = "straight" },
                    { time = 4.0,  enemy = "zigzag", y = 300, pattern = "zigzag" },
                    { time = 5.0,  enemy = "zigzag", y = 500, pattern = "zigzag" },
                    { time = 7.0,  enemy = "basic", y = 150, pattern = "straight" },
                    { time = 7.5,  enemy = "basic", y = 250, pattern = "straight" },
                    { time = 8.0,  enemy = "basic", y = 350, pattern = "straight" },
                    { time = 10.0, enemy = "sinewave", y = 400, pattern = "sinewave" },
                    { time = 13.0, enemy = "basic", y = 300, pattern = "straight", count = 5, spacing = 0.3 },
                    { time = 18.0, enemy = "shooter", y = 500, pattern = "straight" },
                    { time = 20.0, enemy = "zigzag", y = 200, pattern = "zigzag" },
                    { time = 20.0, enemy = "zigzag", y = 600, pattern = "zigzag" },
                },
                
                reward = { type = "weapon_upgrade", y = 400 }
            },
            
            -- Wave 2: Increased pressure
            {
                name = "Pressure",
                startTime = 28.0,
                duration = 30.0,
                
                spawns = {
                    { time = 0.0,  enemy = "basic", y = 150, pattern = "straight" },
                    { time = 0.3,  enemy = "basic", y = 300, pattern = "straight" },
                    { time = 0.6,  enemy = "basic", y = 450, pattern = "straight" },
                    { time = 0.9,  enemy = "basic", y = 600, pattern = "straight" },
                    { time = 3.0,  enemy = "shooter", y = 300, pattern = "straight" },
                    { time = 3.5,  enemy = "shooter", y = 600, pattern = "straight" },
                    { time = 6.0,  enemy = "zigzag", y = 200, pattern = "zigzag" },
                    { time = 6.0,  enemy = "zigzag", y = 400, pattern = "zigzag" },
                    { time = 6.0,  enemy = "zigzag", y = 600, pattern = "zigzag" },
                    { time = 10.0, enemy = "kamikaze", y = 540, pattern = "chase" },
                    { time = 12.0, enemy = "sinewave", y = 300, pattern = "sinewave" },
                    { time = 12.0, enemy = "sinewave", y = 700, pattern = "sinewave" },
                    { time = 15.0, enemy = "carrier", y = 450, pattern = "straight" },
                    { time = 18.0, enemy = "spreader", y = 400, pattern = "sinewave" },
                    { time = 22.0, enemy = "basic", y = 200, pattern = "straight", count = 8, spacing = 0.2 },
                    { time = 26.0, enemy = "shooter", y = 300, pattern = "straight" },
                    { time = 26.0, enemy = "shooter", y = 600, pattern = "straight" },
                },
                
                reward = { type = "speed_boost", y = 300 }
            },
            
            -- Wave 3: Elite enemies
            {
                name = "Elite Squad",
                startTime = 62.0,
                duration = 35.0,
                
                spawns = {
                    { time = 0.0,  enemy = "elite_fighter", y = 300, pattern = "evasive" },
                    { time = 2.0,  enemy = "elite_fighter", y = 600, pattern = "evasive" },
                    { time = 5.0,  enemy = "armored", y = 450, pattern = "straight" },
                    { time = 8.0,  enemy = "shooter", y = 200, pattern = "straight" },
                    { time = 8.0,  enemy = "shooter", y = 700, pattern = "straight" },
                    { time = 12.0, enemy = "turret", y = 150, pattern = "stationary" },
                    { time = 12.0, enemy = "turret", y = 850, pattern = "stationary" },
                    { time = 15.0, enemy = "zigzag", y = 300, pattern = "zigzag", count = 4, spacing = 0.4 },
                    { time = 20.0, enemy = "formation_leader", y = 450, pattern = "hover" },
                    { time = 25.0, enemy = "kamikaze", y = 200, pattern = "chase" },
                    { time = 25.5, enemy = "kamikaze", y = 400, pattern = "chase" },
                    { time = 26.0, enemy = "kamikaze", y = 600, pattern = "chase" },
                    { time = 30.0, enemy = "shielded", y = 450, pattern = "zigzag" },
                },
                
                reward = { type = "laser_weapon", y = 500 }
            },
            
            -- Wave 4: Pre-boss wave
            {
                name = "Final Assault",
                startTime = 100.0,
                duration = 25.0,
                
                spawns = {
                    { time = 0.0,  enemy = "basic", y = 150, pattern = "straight", count = 6, spacing = 0.15 },
                    { time = 0.0,  enemy = "basic", y = 850, pattern = "straight", count = 6, spacing = 0.15 },
                    { time = 4.0,  enemy = "shooter", y = 300, pattern = "straight" },
                    { time = 4.0,  enemy = "shooter", y = 600, pattern = "straight" },
                    { time = 8.0,  enemy = "spreader", y = 450, pattern = "sinewave" },
                    { time = 12.0, enemy = "elite_fighter", y = 250, pattern = "evasive" },
                    { time = 12.0, enemy = "elite_fighter", y = 650, pattern = "evasive" },
                    { time = 16.0, enemy = "armored", y = 400, pattern = "straight" },
                    { time = 20.0, enemy = "carrier", y = 540, pattern = "straight" },
                },
                
                reward = { type = "health_restore", y = 400 }
            },
            
            -- Boss Wave
            {
                name = "BOSS: Dobkeratops",
                startTime = 128.0,
                duration = 60.0,
                isBossWave = true,
                
                boss = "stage1_boss",
                
                -- No regular spawns during boss
                spawns = {}
            }
        },
        
        -- Stage completion
        completionBonus = 5000,
        perfectBonus = 10000,
        speedBonusTime = 120.0,
        speedBonus = 3000
    },
    
    -- ========================================================================
    -- STAGE 2 - ASTEROID BELT
    -- ========================================================================
    stage2 = {
        name = "Asteroid Belt",
        description = "Navigate through the deadly asteroid field",
        stageNumber = 2,
        
        background = {
            texture = "backgrounds/stage2_bg.png",
            scrollSpeed = 250
        },
        
        music = "stage2_bgm",
        bossMusic = "boss_bgm",
        
        duration = 200.0,
        
        waves = {
            {
                name = "Asteroid Field",
                startTime = 0.0,
                duration = 30.0,
                
                spawns = {
                    { time = 1.0,  enemy = "sinewave", y = 300, pattern = "sinewave" },
                    { time = 1.0,  enemy = "sinewave", y = 600, pattern = "sinewave" },
                    { time = 4.0,  enemy = "kamikaze", y = 400, pattern = "chase" },
                    { time = 6.0,  enemy = "basic", y = 200, pattern = "straight", count = 4, spacing = 0.25 },
                    { time = 10.0, enemy = "turret", y = 180, pattern = "stationary" },
                    { time = 10.0, enemy = "turret", y = 820, pattern = "stationary" },
                    { time = 14.0, enemy = "shooter", y = 450, pattern = "sinewave" },
                    { time = 18.0, enemy = "zigzag", y = 300, pattern = "zigzag", count = 5, spacing = 0.3 },
                    { time = 24.0, enemy = "armored", y = 500, pattern = "straight" },
                },
                
                reward = { type = "spread_weapon", y = 350 }
            },
            
            {
                name = "Mining Facility",
                startTime = 35.0,
                duration = 35.0,
                
                spawns = {
                    { time = 0.0,  enemy = "turret", y = 200, pattern = "stationary" },
                    { time = 0.0,  enemy = "turret", y = 500, pattern = "stationary" },
                    { time = 0.0,  enemy = "turret", y = 800, pattern = "stationary" },
                    { time = 3.0,  enemy = "shooter", y = 350, pattern = "straight" },
                    { time = 3.0,  enemy = "shooter", y = 650, pattern = "straight" },
                    { time = 7.0,  enemy = "elite_fighter", y = 400, pattern = "evasive" },
                    { time = 10.0, enemy = "spreader", y = 300, pattern = "sinewave" },
                    { time = 10.0, enemy = "spreader", y = 700, pattern = "sinewave" },
                    { time = 15.0, enemy = "formation_leader", y = 500, pattern = "hover" },
                    { time = 20.0, enemy = "shielded", y = 350, pattern = "straight" },
                    { time = 20.0, enemy = "shielded", y = 650, pattern = "straight" },
                    { time = 25.0, enemy = "kamikaze", y = 300, pattern = "chase", count = 3, spacing = 0.5 },
                    { time = 30.0, enemy = "carrier", y = 500, pattern = "straight" },
                },
                
                reward = { type = "force_pod", y = 450 }
            },
            
            {
                name = "Deep Space",
                startTime = 75.0,
                duration = 40.0,
                
                spawns = {
                    { time = 0.0,  enemy = "elite_fighter", y = 250, pattern = "evasive" },
                    { time = 0.0,  enemy = "elite_fighter", y = 750, pattern = "evasive" },
                    { time = 5.0,  enemy = "armored", y = 400, pattern = "straight" },
                    { time = 5.0,  enemy = "armored", y = 600, pattern = "straight" },
                    { time = 10.0, enemy = "shooter", y = 200, pattern = "straight", count = 3, spacing = 1.0 },
                    { time = 10.0, enemy = "shooter", y = 800, pattern = "straight", count = 3, spacing = 1.0 },
                    { time = 18.0, enemy = "spreader", y = 500, pattern = "sinewave" },
                    { time = 22.0, enemy = "formation_leader", y = 300, pattern = "hover" },
                    { time = 22.0, enemy = "formation_leader", y = 700, pattern = "hover" },
                    { time = 30.0, enemy = "shielded", y = 500, pattern = "zigzag" },
                    { time = 35.0, enemy = "carrier", y = 400, pattern = "straight" },
                },
                
                reward = { type = "extra_life", y = 500 }
            },
            
            {
                name = "BOSS: Gomander",
                startTime = 120.0,
                duration = 80.0,
                isBossWave = true,
                
                boss = "stage2_boss",
                spawns = {}
            }
        },
        
        completionBonus = 7500,
        perfectBonus = 15000,
        speedBonusTime = 140.0,
        speedBonus = 5000
    },
    
    -- ========================================================================
    -- STAGE 3 - WARSHIP ASSAULT
    -- ========================================================================
    stage3 = {
        name = "Warship Assault",
        description = "Destroy the Bydo battleship",
        stageNumber = 3,
        
        background = {
            texture = "backgrounds/stage3_bg.png",
            scrollSpeed = 150
        },
        
        music = "stage3_bgm",
        bossMusic = "boss_bgm",
        
        duration = 240.0,
        
        waves = {
            {
                name = "Outer Defenses",
                startTime = 0.0,
                duration = 40.0,
                
                spawns = {
                    { time = 0.0,  enemy = "turret", y = 150, pattern = "stationary" },
                    { time = 0.0,  enemy = "turret", y = 350, pattern = "stationary" },
                    { time = 0.0,  enemy = "turret", y = 550, pattern = "stationary" },
                    { time = 0.0,  enemy = "turret", y = 750, pattern = "stationary" },
                    { time = 0.0,  enemy = "turret", y = 950, pattern = "stationary" },
                    { time = 5.0,  enemy = "elite_fighter", y = 300, pattern = "evasive" },
                    { time = 5.0,  enemy = "elite_fighter", y = 700, pattern = "evasive" },
                    { time = 10.0, enemy = "shooter", y = 250, pattern = "straight", count = 4, spacing = 0.5 },
                    { time = 15.0, enemy = "armored", y = 500, pattern = "straight" },
                    { time = 20.0, enemy = "spreader", y = 300, pattern = "sinewave" },
                    { time = 20.0, enemy = "spreader", y = 700, pattern = "sinewave" },
                    { time = 25.0, enemy = "kamikaze", y = 400, pattern = "chase", count = 5, spacing = 0.3 },
                    { time = 32.0, enemy = "shielded", y = 500, pattern = "straight" },
                    { time = 35.0, enemy = "carrier", y = 300, pattern = "straight" },
                },
                
                reward = { type = "homing_weapon", y = 400 }
            },
            
            {
                name = "Inner Hull",
                startTime = 45.0,
                duration = 45.0,
                
                spawns = {
                    { time = 0.0,  enemy = "formation_leader", y = 500, pattern = "hover" },
                    { time = 5.0,  enemy = "turret", y = 200, pattern = "stationary" },
                    { time = 5.0,  enemy = "turret", y = 800, pattern = "stationary" },
                    { time = 10.0, enemy = "elite_fighter", y = 350, pattern = "evasive", count = 3, spacing = 0.8 },
                    { time = 18.0, enemy = "armored", y = 300, pattern = "straight" },
                    { time = 18.0, enemy = "armored", y = 700, pattern = "straight" },
                    { time = 25.0, enemy = "spreader", y = 500, pattern = "zigzag" },
                    { time = 30.0, enemy = "shielded", y = 350, pattern = "straight" },
                    { time = 30.0, enemy = "shielded", y = 650, pattern = "straight" },
                    { time = 38.0, enemy = "formation_leader", y = 300, pattern = "hover" },
                    { time = 38.0, enemy = "formation_leader", y = 700, pattern = "hover" },
                },
                
                reward = { type = "option", y = 500 }
            },
            
            {
                name = "Bridge Approach",
                startTime = 95.0,
                duration = 35.0,
                
                spawns = {
                    { time = 0.0,  enemy = "elite_fighter", y = 200, pattern = "evasive" },
                    { time = 0.0,  enemy = "elite_fighter", y = 500, pattern = "evasive" },
                    { time = 0.0,  enemy = "elite_fighter", y = 800, pattern = "evasive" },
                    { time = 8.0,  enemy = "armored", y = 350, pattern = "straight" },
                    { time = 8.0,  enemy = "armored", y = 650, pattern = "straight" },
                    { time = 15.0, enemy = "turret", y = 300, pattern = "stationary" },
                    { time = 15.0, enemy = "turret", y = 700, pattern = "stationary" },
                    { time = 20.0, enemy = "shielded", y = 500, pattern = "sinewave" },
                    { time = 25.0, enemy = "kamikaze", y = 300, pattern = "chase", count = 4, spacing = 0.4 },
                    { time = 30.0, enemy = "carrier", y = 500, pattern = "straight" },
                },
                
                reward = { type = "bomb", y = 450 }
            },
            
            {
                name = "BOSS: Battleship Green",
                startTime = 135.0,
                duration = 105.0,
                isBossWave = true,
                
                boss = "stage3_boss",
                spawns = {}
            }
        },
        
        completionBonus = 10000,
        perfectBonus = 20000,
        speedBonusTime = 180.0,
        speedBonus = 7500
    }
}

-- ============================================================================
-- WAVE GENERATION HELPERS
-- ============================================================================

WavePatterns = {
    -- Formation patterns
    formations = {
        line = function(baseY, count, ySpacing)
            local spawns = {}
            for i = 1, count do
                table.insert(spawns, { y = baseY + (i - 1) * ySpacing })
            end
            return spawns
        end,
        
        v_formation = function(centerY, count, xSpacing, ySpacing)
            local spawns = {}
            local half = math.floor(count / 2)
            for i = 1, count do
                local offset = i - half - 1
                table.insert(spawns, {
                    xOffset = math.abs(offset) * xSpacing,
                    y = centerY + offset * ySpacing
                })
            end
            return spawns
        end,
        
        circle = function(centerY, count, radius)
            local spawns = {}
            for i = 1, count do
                local angle = (i - 1) * (360 / count)
                table.insert(spawns, {
                    xOffset = math.cos(math.rad(angle)) * radius,
                    y = centerY + math.sin(math.rad(angle)) * radius
                })
            end
            return spawns
        end
    },
    
    -- Spawn timing patterns
    timing = {
        burst = function(baseTime, count, interval)
            local times = {}
            for i = 1, count do
                table.insert(times, baseTime + (i - 1) * interval)
            end
            return times
        end,
        
        staggered = function(baseTime, count, interval, groupSize)
            local times = {}
            for i = 1, count do
                local groupIndex = math.floor((i - 1) / groupSize)
                table.insert(times, baseTime + groupIndex * interval)
            end
            return times
        end
    }
}

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Get stage config
function GetStage(stageNumber)
    local stageName = "stage" .. stageNumber
    return StagesConfig[stageName]
end

-- Get wave from stage
function GetWave(stageNumber, waveIndex)
    local stage = GetStage(stageNumber)
    if stage and stage.waves then
        return stage.waves[waveIndex]
    end
    return nil
end

-- Get total wave count for stage
function GetWaveCount(stageNumber)
    local stage = GetStage(stageNumber)
    if stage and stage.waves then
        return #stage.waves
    end
    return 0
end

-- Get boss wave for stage
function GetBossWave(stageNumber)
    local stage = GetStage(stageNumber)
    if not stage then return nil end
    
    for _, wave in ipairs(stage.waves) do
        if wave.isBossWave then
            return wave
        end
    end
    return nil
end

-- Get spawns for current time
function GetSpawnsAtTime(stageNumber, stageTime)
    local stage = GetStage(stageNumber)
    if not stage then return {} end
    
    local spawns = {}
    
    for _, wave in ipairs(stage.waves) do
        if stageTime >= wave.startTime and stageTime < wave.startTime + wave.duration then
            local waveTime = stageTime - wave.startTime
            
            for _, spawn in ipairs(wave.spawns) do
                -- Check if this spawn should happen now
                if spawn.time <= waveTime and spawn.time > waveTime - 0.016 then
                    if spawn.count and spawn.count > 1 then
                        -- Multiple spawns with spacing
                        for i = 1, spawn.count do
                            if spawn.time + (i - 1) * (spawn.spacing or 0.3) <= waveTime and
                               spawn.time + (i - 1) * (spawn.spacing or 0.3) > waveTime - 0.016 then
                                local s = {}
                                for k, v in pairs(spawn) do s[k] = v end
                                s.count = nil
                                s.spacing = nil
                                table.insert(spawns, s)
                            end
                        end
                    else
                        table.insert(spawns, spawn)
                    end
                end
            end
        end
    end
    
    return spawns
end

-- Get active wave
function GetActiveWave(stageNumber, stageTime)
    local stage = GetStage(stageNumber)
    if not stage then return nil, 0 end
    
    for i, wave in ipairs(stage.waves) do
        if stageTime >= wave.startTime and stageTime < wave.startTime + wave.duration then
            return wave, i
        end
    end
    
    return nil, 0
end

print("[StagesConfig] Loaded stage configurations")
