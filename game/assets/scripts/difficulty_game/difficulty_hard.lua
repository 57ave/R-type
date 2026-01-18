-- ============================================
-- DIFFICULTY SETTINGS - HARD
-- ============================================
-- Challenging difficulty for experienced players
-- ============================================

DifficultySettings = {
    name = "hard",
    displayName = "Hard",
    
    -- ========================================
    -- PLAYER SETTINGS
    -- ========================================
    player = {
        health = 2,                 -- Less starting health
        maxHealth = 2,
        damage = 0.9,               -- 10% less damage
        speed = 1.0,                -- Normal movement
        fireRate = 0.9,             -- 10% slower shooting
        invincibilityDuration = 1.0 -- Shorter invincibility after hit
    },
    
    -- ========================================
    -- ENEMY SETTINGS
    -- ========================================
    enemies = {
        healthMultiplier = 1.3,     -- 30% more health
        damageMultiplier = 1.2,     -- 20% more damage
        speedMultiplier = 1.2,      -- 20% faster
        fireRateMultiplier = 1.3,   -- 30% faster shooting
        accuracyMultiplier = 1.2    -- More accurate
    },
    
    -- ========================================
    -- GAMEPLAY SETTINGS
    -- ========================================
    gameplay = {
        scrollSpeed = 1.1,          -- 10% faster scroll
        enemySpawnRate = 1.3,       -- 30% more enemies
        waveInterval = 0.8,         -- 20% less time between waves
        bossHealthMultiplier = 1.5, -- Bosses have 50% more health
        powerupDropRate = 0.7       -- 30% fewer power-ups
    },
    
    -- ========================================
    -- SCORING
    -- ========================================
    scoring = {
        scoreMultiplier = 1.5       -- 50% more score (harder = more reward)
    }
}

--- Apply difficulty settings to game
-- @param game Game instance reference
function ApplyDifficulty(game)
    print("[Difficulty] Applying HARD difficulty settings")
    
    if game and game.SetDifficultyMultipliers then
        game:SetDifficultyMultipliers(
            DifficultySettings.enemies.healthMultiplier,
            DifficultySettings.enemies.damageMultiplier,
            DifficultySettings.enemies.speedMultiplier
        )
    end
end

print("[Difficulty] ✓ HARD settings loaded")
