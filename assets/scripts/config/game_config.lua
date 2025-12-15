-- assets/scripts/config/game_config.lua
-- Global game configuration

GameConfig = {
    window = {
        width = 1280,
        height = 720,
        title = "R-Type",
        fullscreen = false,
        vsync = true
    },
    
    player = {
        startX = 100,
        startY = 360,
        health = 100,
        speed = 400,
        fireRate = 0.2  -- seconds between shots
    },
    
    gameplay = {
        scrollSpeed = 100,
        enemySpawnX = 800,
        bulletSpeed = 600,
        enemyBulletSpeed = 400
    },
    
    difficulty = {
        easy = {
            enemyHealth = 0.7,
            enemyDamage = 0.5,
            enemySpeed = 0.8,
            spawnRate = 1.2
        },
        normal = {
            enemyHealth = 1.0,
            enemyDamage = 1.0,
            enemySpeed = 1.0,
            spawnRate = 1.0
        },
        hard = {
            enemyHealth = 1.5,
            enemyDamage = 1.5,
            enemySpeed = 1.3,
            spawnRate = 0.7
        }
    }
}

-- Helper function to get current difficulty settings
function GetDifficulty(level)
    level = level or "normal"
    return GameConfig.difficulty[level]
end
