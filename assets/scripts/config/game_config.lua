-- assets/scripts/config/game_config.lua
-- Global game configuration

GameConfig = {
    window = {
        width = 1920,
        height = 1080,
        title = "R-Type - Lua Edition",
        fullscreen = false,
        vsync = true
    },
    
    player = {
        startX = 100,
        startY = 400,
        health = 100,
        speed = 500,
        fireRate = 0.2,  -- seconds between shots
        chargeTime = 0.1,  -- minimum time to charge
        maxChargeTime = 1.0
    },
    
    enemies = {
        basic = {
            health = 30,
            speed = 200,
            damage = 10,
            scoreValue = 100
        },
        zigzag = {
            health = 50,
            speed = 250,
            damage = 15,
            scoreValue = 200,
            amplitude = 100,
            frequency = 2.0
        },
        sinewave = {
            health = 60,
            speed = 180,
            damage = 20,
            scoreValue = 250,
            amplitude = 150,
            frequency = 1.5
        },
        kamikaze = {
            health = 20,
            speed = 400,
            damage = 30,
            scoreValue = 150
        },
        turret = {
            health = 80,
            speed = 0,
            damage = 25,
            scoreValue = 300,
            shootInterval = 2.0
        },
        boss = {
            health = 500,
            speed = 100,
            damage = 40,
            scoreValue = 5000,
            shootInterval = 1.0
        }
    },
    
    projectiles = {
        playerNormal = {
            speed = 800,
            damage = 10,
            lifetime = 5.0
        },
        playerCharged = {
            speed = 600,
            damage = {25, 50, 75, 100, 150},  -- Per charge level
            lifetime = 5.0
        },
        enemyBasic = {
            speed = 400,
            damage = 15,
            lifetime = 5.0
        }
    },
    
    gameplay = {
        scrollSpeed = 200,
        spawnX = 1970,  -- Right edge + offset
        minY = 100,
        maxY = 980
    },
    
    damage = {
        playerBullet = 10,
        enemyBullet = 15,
        enemyContact = 20
    },
    
    difficulty = {
        easy = {
            enemyHealth = 0.7,
            enemyDamage = 0.5,
            enemySpeed = 0.8,
            spawnRate = 1.5
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
        },
        insane = {
            enemyHealth = 2.0,
            enemyDamage = 2.0,
            enemySpeed = 1.5,
            spawnRate = 0.5
        }
    },
    
    -- Current difficulty setting
    currentDifficulty = "normal"
}

-- Helper function to get current difficulty settings
function GetDifficulty(level)
    level = level or GameConfig.currentDifficulty
    return GameConfig.difficulty[level]
end

-- Helper function to apply difficulty scaling
function ApplyDifficultyScale(baseValue, statType)
    local diff = GetDifficulty()
    
    if statType == "health" then
        return baseValue * diff.enemyHealth
    elseif statType == "damage" then
        return baseValue * diff.enemyDamage
    elseif statType == "speed" then
        return baseValue * diff.enemySpeed
    elseif statType == "spawnRate" then
        return baseValue * diff.spawnRate
    end
    
    return baseValue
end

print("[GameConfig] Loaded - Difficulty: " .. GameConfig.currentDifficulty)

-- ==========================================================================
-- ASSETS PATHS (used by C++ to load textures/sounds/fonts)
-- Put all asset relative paths here so C++ is fully data-driven
-- ==========================================================================

Assets = {
    background = "game/assets/background.png",
    base = "game/assets/",
    players = {
        player = "game/assets/players/r-typesheet42.png",
        missile = "game/assets/players/r-typesheet1.png"
    },
    enemies = {
        bullets = "game/assets/enemies/enemy_bullets.png",
        explosion = "game/assets/enemies/r-typesheet44.png"
    },
    fonts = {
        default = "game/assets/fonts/Roboto-Regular.ttf"
    },
    sounds = {
        base = "game/assets/sounds/",
        shoot = "game/assets/vfx/shoot.ogg",
        menu = "game/assets/sounds/Title.ogg"
    }
}

-- Optional: script paths and other assets
Assets.scripts = {
    init = "assets/scripts/init.lua",
    audio_config = "game/assets/scripts/config/audio_config.lua",
    ui_init = "game/assets/scripts/ui_init.lua"
}
-- spawn system script used by the engine (path provided here so C++ doesn't hardcode it)
Assets.scripts.spawn_system = "assets/scripts/systems/spawn_system.lua"

-- Additional paths
Assets.config = {
    user_settings = "game/assets/config/user_settings.json"
}

Assets.scripts.difficulty_base = "game/assets/scripts/difficulty_game/"

print("[Assets] Loaded asset paths from game_config.lua")
