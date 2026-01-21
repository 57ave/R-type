-- ============================================================================
-- MASTER CONFIGURATION LOADER
-- Loads all game configuration files in the correct order
-- ============================================================================

print("========================================")
print("  R-TYPE CONFIGURATION LOADER")
print("========================================")

-- Base path for scripts
local scriptPath = "assets/scripts/"

-- Configuration load order (dependencies first)
local configFiles = {
    "gameplay_config.lua",   -- Core gameplay settings
    "weapons_config.lua",    -- All weapon definitions
    "enemies_config.lua",    -- All enemy types
    "bosses_config.lua",     -- Boss configurations
    "powerups_config.lua",   -- Power-ups and attachments
    "stages_config.lua"      -- Stages and waves
}

-- Track loaded configs
LoadedConfigs = {}

-- Load each config file
for _, file in ipairs(configFiles) do
    local fullPath = scriptPath .. file
    print("[Config] Loading: " .. file)
    
    local success, err = pcall(function()
        dofile(fullPath)
    end)
    
    if success then
        table.insert(LoadedConfigs, file)
        print("[Config] ✓ Loaded: " .. file)
    else
        print("[Config] ✗ Failed to load: " .. file)
        print("[Config] Error: " .. tostring(err))
    end
end

print("----------------------------------------")
print("[Config] Loaded " .. #LoadedConfigs .. "/" .. #configFiles .. " config files")
print("========================================")

-- ============================================================================
-- GLOBAL GAME API
-- High-level functions for the C++ code to call
-- ============================================================================

GameAPI = {
    -- Get player starting configuration
    GetPlayerConfig = function()
        return GameplayConfig.player
    end,
    
    -- Get current difficulty multipliers
    GetDifficultyMultipliers = function()
        return GetCurrentDifficulty()
    end,
    
    -- Get enemy configuration (scaled for difficulty)
    GetEnemyConfig = function(enemyType)
        return GetEnemyScaled(enemyType)
    end,
    
    -- Get weapon configuration at level
    GetWeaponConfig = function(weaponName, level)
        return GetWeaponAtLevel(weaponName, level or 1)
    end,
    
    -- Get boss configuration
    GetBossConfig = function(bossType)
        return GetBossScaled(bossType)
    end,
    
    -- Get power-up configuration
    GetPowerUpConfig = function(powerUpType)
        return GetPowerUp(powerUpType)
    end,
    
    -- Get attachment configuration
    GetAttachmentConfig = function(attachmentType)
        return GetAttachment(attachmentType)
    end,
    
    -- Get stage configuration
    GetStageConfig = function(stageNumber)
        return GetStage(stageNumber)
    end,
    
    -- Get spawns for current time
    GetCurrentSpawns = function(stageNumber, time)
        return GetSpawnsAtTime(stageNumber, time)
    end,
    
    -- Get active wave info
    GetActiveWaveInfo = function(stageNumber, time)
        local wave, index = GetActiveWave(stageNumber, time)
        if wave then
            return {
                name = wave.name,
                index = index,
                isBossWave = wave.isBossWave or false,
                boss = wave.boss,
                startTime = wave.startTime,
                duration = wave.duration
            }
        end
        return nil
    end,
    
    -- Set difficulty
    SetDifficulty = function(difficulty)
        if GameplayConfig.difficulty[difficulty] then
            GameplayConfig.currentDifficulty = difficulty
            print("[GameAPI] Difficulty set to: " .. difficulty)
            return true
        end
        return false
    end,
    
    -- Get all available difficulties
    GetDifficulties = function()
        local diffs = {}
        for name, _ in pairs(GameplayConfig.difficulty) do
            table.insert(diffs, name)
        end
        return diffs
    end,
    
    -- Get all weapon types
    GetAllWeaponTypes = function()
        local weapons = {}
        for name, weapon in pairs(WeaponsConfig) do
            if weapon.category == "primary" or weapon.category == "secondary" then
                table.insert(weapons, name)
            end
        end
        return weapons
    end,
    
    -- Get all enemy types
    GetAllEnemyTypes = function()
        return GetAllEnemyTypes()
    end,
    
    -- Calculate charge level
    GetChargeLevel = function(weaponName, chargeTime)
        return GetChargeLevel(weaponName, chargeTime)
    end,
    
    -- Get random drop from enemy
    GetEnemyDrop = function(enemyType)
        local enemy = EnemiesConfig[enemyType]
        if not enemy then return nil end
        return GetRandomDrop(enemy.dropTable, enemy.dropChance)
    end
}

-- ============================================================================
-- SPAWN MANAGER API
-- For the wave/spawn system
-- ============================================================================

SpawnManager = {
    currentStage = 1,
    currentTime = 0,
    waveIndex = 1,
    isPaused = false,
    bossActive = false,
    
    -- Initialize for a stage
    StartStage = function(stageNumber)
        SpawnManager.currentStage = stageNumber
        SpawnManager.currentTime = 0
        SpawnManager.waveIndex = 1
        SpawnManager.isPaused = false
        SpawnManager.bossActive = false
        
        local stage = GetStage(stageNumber)
        if stage then
            print("[SpawnManager] Starting Stage " .. stageNumber .. ": " .. stage.name)
            return true
        end
        return false
    end,
    
    -- Update and get spawns
    Update = function(deltaTime)
        if SpawnManager.isPaused then return {} end
        
        SpawnManager.currentTime = SpawnManager.currentTime + deltaTime
        
        return GetSpawnsAtTime(SpawnManager.currentStage, SpawnManager.currentTime)
    end,
    
    -- Get current wave name
    GetCurrentWaveName = function()
        local wave, _ = GetActiveWave(SpawnManager.currentStage, SpawnManager.currentTime)
        if wave then
            return wave.name
        end
        return "Unknown"
    end,
    
    -- Check if boss should spawn
    ShouldSpawnBoss = function()
        if SpawnManager.bossActive then return false end
        
        local wave, _ = GetActiveWave(SpawnManager.currentStage, SpawnManager.currentTime)
        if wave and wave.isBossWave and not SpawnManager.bossActive then
            SpawnManager.bossActive = true
            return wave.boss
        end
        return nil
    end,
    
    -- Pause spawning
    Pause = function()
        SpawnManager.isPaused = true
    end,
    
    -- Resume spawning
    Resume = function()
        SpawnManager.isPaused = false
    end,
    
    -- Get stage progress
    GetProgress = function()
        local stage = GetStage(SpawnManager.currentStage)
        if not stage then return 0 end
        return SpawnManager.currentTime / stage.duration
    end
}

print("[MasterConfig] Game API and Spawn Manager initialized")
print("========================================")
