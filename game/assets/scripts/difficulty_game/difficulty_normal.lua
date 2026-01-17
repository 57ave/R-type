-- ============================================
-- DIFFICULTY SETTINGS - NORMAL
-- ============================================
-- Balanced difficulty for most players
-- ============================================

DifficultySettings = {
    name = "normal",
    displayName = "Normal",
    
    -- ========================================
    -- PLAYER SETTINGS
    -- ========================================
    player = {
        health = 3,                 -- Starting health
        maxHealth = 3,
        damage = 1.0,               -- Normal damage
        speed = 1.0,                -- Normal movement
        fireRate = 1.0,             -- Normal shooting
        invincibilityDuration = 1.5 -- Standard invincibility after hit
    },
    
    -- ========================================
    -- ENEMY SETTINGS
    -- ========================================
    enemies = {
        healthMultiplier = 1.0,     -- Normal health
        damageMultiplier = 1.0,     -- Normal damage
        speedMultiplier = 1.0,      -- Normal speed
        fireRateMultiplier = 1.0,   -- Normal shooting
        accuracyMultiplier = 1.0    -- Normal accuracy
    },
    
    -- ========================================
    -- GAMEPLAY SETTINGS
    -- ========================================
    gameplay = {
        scrollSpeed = 1.0,          -- Normal scroll speed
        enemySpawnRate = 1.0,       -- Normal enemy count
        waveInterval = 1.0,         -- Normal time between waves
        bossHealthMultiplier = 1.0, -- Normal boss health
        powerupDropRate = 1.0       -- Normal power-up rate
    },
    
    -- ========================================
    -- SCORING
    -- ========================================
    scoring = {
        scoreMultiplier = 1.0       -- Normal score
    }
}

--- Apply difficulty settings to game
-- @param game Game instance reference
function ApplyDifficulty(game)
    print("[Difficulty] Applying NORMAL difficulty settings")
    
    if game and game.SetDifficultyMultipliers then
        game:SetDifficultyMultipliers(
            DifficultySettings.enemies.healthMultiplier,
            DifficultySettings.enemies.damageMultiplier,
            DifficultySettings.enemies.speedMultiplier
        )
    end
end

print("[Difficulty] ✓ NORMAL settings loaded")
