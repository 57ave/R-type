-- ============================================================================
-- WAVE MANAGER
-- Manages wave progression, enemy spawning, and wave transitions
-- ============================================================================

WaveManager = {
    -- Current state
    currentStage = 1,
    currentWaveIndex = 0,
    waveInProgress = false,
    stageTime = 0.0,
    waveStartTime = 0.0,
    
    -- Wave UI
    showingWaveAnnounce = false,
    waveAnnounceTimer = 0.0,
    waveAnnounceDuration = 3.0,
    waveAnnounceText = "",
    waveAnnounceFadeIn = 0.5,
    waveAnnounceFadeOut = 0.5,
    
    -- Completion tracking
    currentWaveEnemiesAlive = 0,
    totalWavesCompleted = 0,
    
    -- Boss state
    bossActive = false,
    bossEntity = nil,
    bossHealthPercent = 1.0
}

-- ============================================================================
-- INITIALIZATION
-- ============================================================================

function WaveManager:Init(stageNumber)
    self.currentStage = stageNumber or 1
    self.currentWaveIndex = 0
    self.waveInProgress = false
    self.stageTime = 0.0
    self.waveStartTime = 0.0
    self.showingWaveAnnounce = false
    self.waveAnnounceTimer = 0.0
    self.currentWaveEnemiesAlive = 0
    self.totalWavesCompleted = 0
    self.bossActive = false
    self.bossEntity = nil
    
    print("[WaveManager] Initialized for Stage " .. self.currentStage)
    
    return true
end

-- ============================================================================
-- UPDATE
-- ============================================================================

function WaveManager:Update(deltaTime)
    if not deltaTime then return end
    
    self.stageTime = self.stageTime + deltaTime
    
    -- Update wave announce animation
    if self.showingWaveAnnounce then
        self.waveAnnounceTimer = self.waveAnnounceTimer + deltaTime
        if self.waveAnnounceTimer >= self.waveAnnounceDuration then
            self.showingWaveAnnounce = false
            self.waveAnnounceTimer = 0.0
        end
    end
    
    -- Get current stage configuration
    local stageConfig = self:GetCurrentStageConfig()
    if not stageConfig then
        return
    end
    
    -- Check if we should start the next wave
    if not self.waveInProgress and not self.bossActive then
        local nextWave = self:GetNextWave()
        if nextWave and self.stageTime >= nextWave.startTime then
            self:StartWave(nextWave)
        end
    end
    
    -- Boss-specific updates
    if self.bossActive then
        self:UpdateBoss(deltaTime)
    end
end

-- ============================================================================
-- WAVE MANAGEMENT
-- ============================================================================

function WaveManager:StartWave(waveConfig)
    if not waveConfig then return false end
    
    self.currentWaveIndex = self.currentWaveIndex + 1
    self.waveInProgress = true
    self.waveStartTime = self.stageTime
    self.currentWaveEnemiesAlive = 0
    
    -- Show wave announcement
    self:ShowWaveAnnouncement(waveConfig.name, self.currentWaveIndex)
    
    -- Check if this is a boss wave
    if waveConfig.isBossWave then
        print("[WaveManager] 👾 BOSS WAVE: " .. waveConfig.name)
        self:StartBossWave(waveConfig)
    else
        print("[WaveManager] 🌊 Wave " .. self.currentWaveIndex .. ": " .. waveConfig.name)
        self:ScheduleWaveSpawns(waveConfig)
    end
    
    return true
end

function WaveManager:ScheduleWaveSpawns(waveConfig)
    if not waveConfig.spawns then return end
    
    -- Schedule all enemy spawns for this wave
    for _, spawn in ipairs(waveConfig.spawns) do
        self:ScheduleEnemySpawn(spawn, waveConfig)
    end
    
    print("[WaveManager] Scheduled " .. #waveConfig.spawns .. " spawn events")
end

function WaveManager:ScheduleEnemySpawn(spawnData, waveConfig)
    -- This will be called by C++ spawn system
    -- For now, log the spawn request
    local spawnTime = self.waveStartTime + spawnData.time
    local enemyType = spawnData.enemy
    local yPosition = spawnData.y or 400
    local count = spawnData.count or 1
    local spacing = spawnData.spacing or 0.5
    
    -- Notify C++ to spawn enemy (if binding exists)
    if SpawnEnemy then
        -- Spawn with delay
        local delay = spawnData.time
        if count > 1 then
            -- Spawn multiple enemies with spacing
            for i = 1, count do
                local spawnDelay = delay + ((i - 1) * spacing)
                -- Schedule spawn through C++ binding
                if ScheduleEnemySpawn then
                    ScheduleEnemySpawn(enemyType, yPosition, spawnDelay)
                end
            end
        else
            -- Single enemy spawn
            if ScheduleEnemySpawn then
                ScheduleEnemySpawn(enemyType, yPosition, delay)
            end
        end
    end
end

function WaveManager:CompleteWave()
    if not self.waveInProgress then return end
    
    self.waveInProgress = false
    self.totalWavesCompleted = self.totalWavesCompleted + 1
    
    print("[WaveManager] ✓ Wave " .. self.currentWaveIndex .. " completed!")
    
    -- Show completion message
    self:ShowWaveAnnouncement("WAVE CLEARED", self.currentWaveIndex)
    
    -- Check if stage is complete
    if self:IsStageComplete() then
        self:CompleteStage()
    end
end

function WaveManager:IsWaveComplete()
    -- Wave is complete when all enemies are defeated
    return self.currentWaveEnemiesAlive <= 0 and self.waveInProgress
end

-- ============================================================================
-- BOSS MANAGEMENT
-- ============================================================================

function WaveManager:StartBossWave(waveConfig)
    self.bossActive = true
    
    local bossType = waveConfig.boss
    print("[WaveManager] 💀 Spawning Boss: " .. bossType)
    
    -- Spawn boss through C++ binding
    if SpawnBoss then
        self.bossEntity = SpawnBoss(bossType, 1600, 540)  -- Center-right of screen
    end
    
    -- Change music to boss music
    if Audio and Audio.PlayBossMusic then
        Audio.PlayBossMusic()
    end
end

function WaveManager:UpdateBoss(deltaTime)
    if not self.bossEntity then return end
    
    -- Update boss health percentage (from C++ component)
    if GetEntityHealth then
        local currentHealth, maxHealth = GetEntityHealth(self.bossEntity)
        if currentHealth and maxHealth and maxHealth > 0 then
            self.bossHealthPercent = currentHealth / maxHealth
        end
    end
end

function WaveManager:OnBossDefeated()
    if not self.bossActive then return end
    
    print("[WaveManager] 🏆 BOSS DEFEATED!")
    
    self.bossActive = false
    self.bossEntity = nil
    self.bossHealthPercent = 0.0
    
    -- Complete the wave
    self:CompleteWave()
    
    -- Show victory message
    self:ShowWaveAnnouncement("BOSS DESTROYED", 0)
end

-- ============================================================================
-- STAGE MANAGEMENT
-- ============================================================================

function WaveManager:CompleteStage()
    print("[WaveManager] ========================================")
    print("[WaveManager] 🎉 STAGE " .. self.currentStage .. " COMPLETE!")
    print("[WaveManager] ========================================")
    
    local stageConfig = self:GetCurrentStageConfig()
    if stageConfig then
        -- Apply completion bonuses
        if SoloGameData then
            SoloGameData.score = (SoloGameData.score or 0) + (stageConfig.completionBonus or 0)
            print("[WaveManager] Completion Bonus: " .. (stageConfig.completionBonus or 0))
        end
    end
    
    -- Trigger victory state
    if GameState and GameState.Set then
        GameState.Set("Victory")
    end
end

function WaveManager:IsStageComplete()
    local stageConfig = self:GetCurrentStageConfig()
    if not stageConfig then return false end
    
    -- Stage is complete when all waves are done
    return self.currentWaveIndex >= #stageConfig.waves
end

-- ============================================================================
-- WAVE ANNOUNCEMENTS
-- ============================================================================

function WaveManager:ShowWaveAnnouncement(text, waveNumber)
    if waveNumber and waveNumber > 0 then
        self.waveAnnounceText = "WAVE " .. waveNumber .. "\n" .. text
    else
        self.waveAnnounceText = text
    end
    
    self.showingWaveAnnounce = true
    self.waveAnnounceTimer = 0.0
    
    print("[WaveManager] 📢 " .. text)
end

function WaveManager:GetWaveAnnounceAlpha()
    if not self.showingWaveAnnounce then return 0.0 end
    
    local t = self.waveAnnounceTimer
    local fadeIn = self.waveAnnounceFadeIn
    local fadeOut = self.waveAnnounceFadeOut
    local totalDuration = self.waveAnnounceDuration
    
    -- Fade in
    if t < fadeIn then
        return t / fadeIn
    end
    
    -- Fade out
    if t > totalDuration - fadeOut then
        local fadeTime = totalDuration - t
        return fadeTime / fadeOut
    end
    
    -- Full opacity
    return 1.0
end

-- ============================================================================
-- HELPERS
-- ============================================================================

function WaveManager:GetCurrentStageConfig()
    if not StagesConfig then return nil end
    
    local stageName = "stage" .. self.currentStage
    return StagesConfig[stageName]
end

function WaveManager:GetNextWave()
    local stageConfig = self:GetCurrentStageConfig()
    if not stageConfig or not stageConfig.waves then return nil end
    
    local nextIndex = self.currentWaveIndex + 1
    if nextIndex <= #stageConfig.waves then
        return stageConfig.waves[nextIndex]
    end
    
    return nil
end

function WaveManager:OnEnemySpawned()
    self.currentWaveEnemiesAlive = self.currentWaveEnemiesAlive + 1
end

function WaveManager:OnEnemyKilled()
    self.currentWaveEnemiesAlive = math.max(0, self.currentWaveEnemiesAlive - 1)
    
    -- Check if wave is complete
    if self:IsWaveComplete() then
        self:CompleteWave()
    end
end

-- ============================================================================
-- DEBUGGING
-- ============================================================================

function WaveManager:GetDebugInfo()
    return {
        stage = self.currentStage,
        wave = self.currentWaveIndex,
        stageTime = string.format("%.1f", self.stageTime),
        enemiesAlive = self.currentWaveEnemiesAlive,
        wavesCompleted = self.totalWavesCompleted,
        bossActive = self.bossActive,
        bossHealth = string.format("%.1f%%", self.bossHealthPercent * 100)
    }
end

print("[WaveManager] Wave Manager system loaded")

return WaveManager
