-- ============================================
-- DIFFICULTY SETTINGS - EASY
-- ============================================
-- Relaxed difficulty for casual players
-- ============================================

DifficultySettings = {
    name = "easy",
    displayName = "Easy",
    
    -- ========================================
    -- PLAYER SETTINGS
    -- ========================================
    player = {
        health = 5,                 -- Starting health (more than normal)
        maxHealth = 5,
        damage = 1.2,               -- 20% more damage
        speed = 1.1,                -- 10% faster movement
        fireRate = 1.2,             -- 20% faster shooting
        invincibilityDuration = 2.0 -- Longer invincibility after hit
    },
    
    -- ========================================
    -- ENEMY SETTINGS
    -- ========================================
    enemies = {
        healthMultiplier = 0.7,     -- 30% less health
        damageMultiplier = 0.8,     -- 20% less damage
        speedMultiplier = 0.9,      -- 10% slower
        fireRateMultiplier = 0.7,   -- 30% slower shooting
        accuracyMultiplier = 0.8    -- Less accurate
    },
    
    -- ========================================
    -- GAMEPLAY SETTINGS
    -- ========================================
    gameplay = {
        scrollSpeed = 1.0,          -- Normal scroll speed
        enemySpawnRate = 0.8,       -- 20% fewer enemies
        waveInterval = 1.2,         -- 20% more time between waves
        bossHealthMultiplier = 0.6, -- Bosses have 40% less health
        powerupDropRate = 1.3       -- 30% more power-ups
    },
    
    -- ========================================
    -- SCORING
    -- ========================================
    scoring = {
        scoreMultiplier = 0.8       -- 20% less score (easier = less reward)
    }
}

--- Apply difficulty settings to game
-- @param game Game instance reference
function ApplyDifficulty(game)
    print("[Difficulty] Applying EASY difficulty settings")
    
    if game and game.SetDifficultyMultipliers then
        game:SetDifficultyMultipliers(
            DifficultySettings.enemies.healthMultiplier,
            DifficultySettings.enemies.damageMultiplier,
            DifficultySettings.enemies.speedMultiplier
        )
    end
end

print("[Difficulty] ✓ EASY settings loaded")
