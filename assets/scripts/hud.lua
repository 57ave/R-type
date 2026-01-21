-- ============================================================================
-- IN-GAME HUD (HEADS-UP DISPLAY)
-- Task 17: Global UI improvements
-- ============================================================================

HUD = {
    elements = {},
    isVisible = false,
    currentStats = {
        score = 0,
        wave = 1,
        health = 100,
        maxHealth = 100,
        lives = 3,
        powerup = "None"
    }
}

-- ============================================================================
-- COLORS
-- ============================================================================
local COLORS = {
    WHITE = 0xFFFFFFFF,
    GREEN = 0x00FF00FF,
    YELLOW = 0xFFFF00FF,
    ORANGE = 0xFFA500FF,
    RED = 0xFF0000FF,
    BLUE = 0x00AAFFFF,
    DARK_BG = 0x00000088,
    HEALTH_BG = 0x333333FF,
    HEALTH_GOOD = 0x00FF00FF,
    HEALTH_MID = 0xFFFF00FF,
    HEALTH_LOW = 0xFF0000FF
}

-- ============================================================================
-- INITIALIZATION
-- ============================================================================

function HUD:Init(screenWidth, screenHeight)
    print("[HUD] Initializing in-game HUD...")
    
    screenWidth = screenWidth or 1920
    screenHeight = screenHeight or 1080
    
    local menuGroup = "hud"
    local margin = 20
    local topY = 20
    
    -- ========== TOP-LEFT: SCORE & WAVE ==========
    
    -- Score label
    self.elements.scoreLabel = UI.CreateText({
        x = margin + 100,
        y = topY,
        text = "SCORE:",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    -- Score value
    self.elements.scoreValue = UI.CreateText({
        x = margin + 250,
        y = topY,
        text = "0",
        fontSize = 32,
        color = COLORS.BLUE,
        menuGroup = menuGroup
    })
    
    -- Wave indicator
    self.elements.waveLabel = UI.CreateText({
        x = margin + 100,
        y = topY + 50,
        text = "WAVE:",
        fontSize = 20,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    self.elements.waveValue = UI.CreateText({
        x = margin + 200,
        y = topY + 50,
        text = "1",
        fontSize = 28,
        color = COLORS.YELLOW,
        menuGroup = menuGroup
    })
    
    -- ========== TOP-CENTER: WAVE ANNOUNCEMENT ==========
    
    self.elements.waveAnnounce = UI.CreateText({
        x = screenWidth / 2,
        y = 150,
        text = "",
        fontSize = 72,
        color = COLORS.YELLOW,
        menuGroup = menuGroup
    })
    UI.SetVisible(self.elements.waveAnnounce, false)
    
    -- ========== TOP-RIGHT: HEALTH BAR ==========
    
    local healthBarX = screenWidth - 350
    local healthBarY = topY
    local healthBarWidth = 300
    local healthBarHeight = 30
    
    -- Health label
    self.elements.healthLabel = UI.CreateText({
        x = healthBarX + 150,
        y = healthBarY - 25,
        text = "HEALTH",
        fontSize = 18,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    -- Health bar background
    self.elements.healthBarBg = UI.CreatePanel({
        x = healthBarX,
        y = healthBarY,
        width = healthBarWidth,
        height = healthBarHeight,
        backgroundColor = COLORS.HEALTH_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    -- Health bar foreground (will be resized based on health)
    self.elements.healthBarFg = UI.CreatePanel({
        x = healthBarX + 2,
        y = healthBarY + 2,
        width = healthBarWidth - 4,
        height = healthBarHeight - 4,
        backgroundColor = COLORS.HEALTH_GOOD,
        modal = false,
        menuGroup = menuGroup
    })
    
    -- Health text (percentage)
    self.elements.healthText = UI.CreateText({
        x = healthBarX + 150,
        y = healthBarY + 5,
        text = "100%",
        fontSize = 20,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    -- Lives indicator
    self.elements.livesLabel = UI.CreateText({
        x = healthBarX + 150,
        y = healthBarY + 45,
        text = "♥ Lives: 3",
        fontSize = 20,
        color = COLORS.RED,
        menuGroup = menuGroup
    })
    
    -- ========== BOTTOM-CENTER: POWERUP INDICATOR ==========
    
    self.elements.powerupBg = UI.CreatePanel({
        x = screenWidth / 2 - 150,
        y = screenHeight - 100,
        width = 300,
        height = 60,
        backgroundColor = COLORS.DARK_BG,
        modal = false,
        menuGroup = menuGroup
    })
    UI.SetVisible(self.elements.powerupBg, false)
    
    self.elements.powerupText = UI.CreateText({
        x = screenWidth / 2,
        y = screenHeight - 80,
        text = "⚡ POWER-UP: RAPID FIRE",
        fontSize = 24,
        color = COLORS.ORANGE,
        menuGroup = menuGroup
    })
    UI.SetVisible(self.elements.powerupText, false)
    
    print("[HUD] ✓ HUD initialized with " .. self:CountElements() .. " elements")
    self.isVisible = false
end

-- ============================================================================
-- VISIBILITY CONTROL
-- ============================================================================

function HUD:Show()
    if not self.isVisible then
        UI.ShowMenu("hud")
        self.isVisible = true
        print("[HUD] ✓ HUD shown")
    end
end

function HUD:Hide()
    if self.isVisible then
        UI.HideMenu("hud")
        self.isVisible = false
        print("[HUD] ✓ HUD hidden")
    end
end

-- ============================================================================
-- UPDATE FUNCTIONS
-- ============================================================================

function HUD:UpdateScore(score)
    self.currentStats.score = score
    if self.elements.scoreValue then
        UI.SetText(self.elements.scoreValue, tostring(score))
    end
end

function HUD:UpdateWave(wave)
    self.currentStats.wave = wave
    if self.elements.waveValue then
        UI.SetText(self.elements.waveValue, tostring(wave))
    end
end

function HUD:UpdateHealth(current, max)
    self.currentStats.health = current
    self.currentStats.maxHealth = max or 100
    
    local percentage = (current / self.currentStats.maxHealth) * 100
    percentage = math.max(0, math.min(100, percentage))
    
    -- Update text
    if self.elements.healthText then
        UI.SetText(self.elements.healthText, math.floor(percentage) .. "%")
    end
    
    -- Update bar width
    if self.elements.healthBarFg then
        local maxWidth = 296 -- 300 - 4 (for padding)
        local newWidth = (percentage / 100) * maxWidth
        UI.SetSize(self.elements.healthBarFg, newWidth, 26)
        
        -- Change color based on health
        local color = COLORS.HEALTH_GOOD
        if percentage < 30 then
            color = COLORS.HEALTH_LOW
        elseif percentage < 60 then
            color = COLORS.HEALTH_MID
        end
        UI.SetBackgroundColor(self.elements.healthBarFg, color)
    end
end

function HUD:UpdateLives(lives)
    self.currentStats.lives = lives
    if self.elements.livesLabel then
        UI.SetText(self.elements.livesLabel, "♥ Lives: " .. lives)
    end
end

function HUD:ShowPowerup(powerupName, duration)
    self.currentStats.powerup = powerupName
    
    if self.elements.powerupText then
        UI.SetText(self.elements.powerupText, "⚡ POWER-UP: " .. string.upper(powerupName))
        UI.SetVisible(self.elements.powerupText, true)
    end
    
    if self.elements.powerupBg then
        UI.SetVisible(self.elements.powerupBg, true)
    end
    
    -- Auto-hide after duration
    if duration and duration > 0 then
        -- TODO: Set timer to hide after duration seconds
        -- For now, caller should call HidePowerup() manually
    end
end

function HUD:HidePowerup()
    self.currentStats.powerup = "None"
    
    if self.elements.powerupText then
        UI.SetVisible(self.elements.powerupText, false)
    end
    
    if self.elements.powerupBg then
        UI.SetVisible(self.elements.powerupBg, false)
    end
end

function HUD:ShowWaveAnnouncement(wave)
    if self.elements.waveAnnounce then
        UI.SetText(self.elements.waveAnnounce, "〰️ WAVE " .. wave .. " 〰️")
        UI.SetVisible(self.elements.waveAnnounce, true)
        
        -- Auto-hide after 3 seconds (caller should handle this)
        print("[HUD] ⚡ Wave " .. wave .. " announcement shown")
    end
end

function HUD:HideWaveAnnouncement()
    if self.elements.waveAnnounce then
        UI.SetVisible(self.elements.waveAnnounce, false)
    end
end

-- ============================================================================
-- UTILITY
-- ============================================================================

function HUD:CountElements()
    local count = 0
    for _ in pairs(self.elements) do
        count = count + 1
    end
    return count
end

function HUD:GetCurrentStats()
    return self.currentStats
end

-- ============================================================================
-- DEBUG
-- ============================================================================

function HUD:DebugUpdate()
    -- Test function to cycle through different values
    local testScore = math.random(0, 999999)
    local testWave = math.random(1, 20)
    local testHealth = math.random(0, 100)
    local testLives = math.random(0, 5)
    
    self:UpdateScore(testScore)
    self:UpdateWave(testWave)
    self:UpdateHealth(testHealth, 100)
    self:UpdateLives(testLives)
    
    print("[HUD] Debug update: Score=" .. testScore .. ", Wave=" .. testWave .. ", Health=" .. testHealth .. "%, Lives=" .. testLives)
end

print("[HUD] HUD system loaded")

return HUD
