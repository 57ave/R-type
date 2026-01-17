-- ============================================
-- AUDIO CONFIGURATION - R-TYPE
-- ============================================
-- Central configuration for all audio files
-- Musics are streamed, SFX are preloaded
-- ============================================

AudioConfig = {
    -- ========================================
    -- MUSIC FILES (streamed via sf::Music)
    -- ========================================
    music = {
        -- Menu & UI
        menu = "Title.ogg",
        nameEntry = "NAME ENTRY.ogg",
        credit = "CREDIT.ogg",
        
        -- Game Over / Victory
        gameOver = "THE END OF WAR (GAME OVER).ogg",
        allClear = "LIKE A HERO (ALL STAGE CLEAR).ogg",
        
        -- Stage Musics (1-8)
        stages = {
            "BATTLE THEME (STAGE 1 The Encounter).ogg",
            "MONSTER BEAT (STAGE 2 Life Forms in a Cave).ogg",
            "BATTLE PRESSURE (STAGE 3 Giant Warship).ogg",
            "GRANULATIONS (STAGE 4 A Base on The War Front).ogg",
            "MONSTER LURKING IN THE CAVE (STAGE 5 The Den).ogg",
            "SCRAMBLE CROSSROAD (STAGE 6 Transport System).ogg",
            "DREAM ISLAND (STAGE 7 A City in Decay).ogg",
            "WOMB (STAGE 8 A Star Occupied by The Bydo Empire).ogg"
        },
        
        -- Event Musics
        boss = "BOSS THEME.ogg",
        stageClear = "RETURN IN TRIUMPH (STAGE CLEAR).ogg"
    },
    
    -- ========================================
    -- SOUND EFFECTS (preloaded via sf::Sound)
    -- ========================================
    sfx = {
        -- Player actions
        playerShoot = "shoot.ogg",
        
        -- Enemy actions
        enemyLaser = "laser_bot.ogg",
        enemyMultiLaser = "multi_laser_bot.ogg",
        
        -- Combat events
        explosion = "Boom.ogg",
        damage = "damage.ogg",
        
        -- Collectibles (using existing sounds)
        coin = "damage.ogg",      -- TODO: Add proper coin sound
        powerup = "damage.ogg"    -- TODO: Add proper powerup sound
    },
    
    -- ========================================
    -- DEFAULT VOLUMES (0-100)
    -- ========================================
    volumes = {
        musicDefault = 70,
        sfxDefault = 80
    },
    
    -- ========================================
    -- RELATIVE VOLUME MULTIPLIERS FOR SFX
    -- ========================================
    -- These multiply the base SFX volume
    sfxVolumes = {
        playerShoot = 1.0,
        enemyLaser = 0.8,
        enemyMultiLaser = 0.9,
        explosionPlayer = 1.2,
        explosionEnemy = 1.0,
        explosionBoss = 1.5,
        damage = 1.1,
        coin = 0.9,
        powerup = 0.7
    },
    
    -- ========================================
    -- TRANSITION SETTINGS
    -- ========================================
    transitions = {
        fadeDuration = 1.0,         -- Seconds for music fade in/out
        fastFadeDuration = 0.5,     -- Fast fade for game over
        crossfade = true            -- Enable crossfade between tracks
    },
    
    -- ========================================
    -- PATHS
    -- ========================================
    paths = {
        music = "game/assets/sounds/",
        sfx = "game/assets/vfx/"
    }
}

-- ========================================
-- HELPER FUNCTIONS
-- ========================================

--- Get the music filename for a specific stage
-- @param stageNumber Stage number (1-8)
-- @return Music filename or first stage if invalid
function GetStageMusicPath(stageNumber)
    if stageNumber < 1 or stageNumber > 8 then
        print("[AudioConfig] Warning: Invalid stage number " .. tostring(stageNumber) .. ", defaulting to stage 1")
        return AudioConfig.music.stages[1]
    end
    return AudioConfig.music.stages[stageNumber]
end

--- Get the full path for a music file
-- @param musicName Name of the music file
-- @return Full path to the music file
function GetMusicFullPath(musicName)
    return AudioConfig.paths.music .. musicName
end

--- Get the full path for a SFX file
-- @param sfxName Name of the SFX file
-- @return Full path to the SFX file
function GetSFXFullPath(sfxName)
    return AudioConfig.paths.sfx .. sfxName
end

--- Get volume multiplier for a specific SFX
-- @param sfxType Type of SFX (e.g., "playerShoot", "damage")
-- @return Volume multiplier (defaults to 1.0 if not found)
function GetSFXVolumeMultiplier(sfxType)
    return AudioConfig.sfxVolumes[sfxType] or 1.0
end

--- Get the music for a game state
-- @param gameState Current game state string
-- @param stageNumber Optional stage number for Playing state
-- @return Music filename for the state
function GetMusicForState(gameState, stageNumber)
    if gameState == "MainMenu" or gameState == "menu" then
        return AudioConfig.music.menu
    elseif gameState == "CreateRoom" then
        return AudioConfig.music.nameEntry
    elseif gameState == "Playing" then
        return GetStageMusicPath(stageNumber or 1)
    elseif gameState == "GameOver" then
        return AudioConfig.music.gameOver
    elseif gameState == "Victory" or gameState == "AllClear" then
        return AudioConfig.music.allClear
    elseif gameState == "Credits" then
        return AudioConfig.music.credit
    else
        return AudioConfig.music.menu
    end
end

-- ========================================
-- INITIALIZATION LOG
-- ========================================
print("[AudioConfig] ✓ Loaded with " .. #AudioConfig.music.stages .. " stage musics")
print("[AudioConfig]   Music path: " .. AudioConfig.paths.music)
print("[AudioConfig]   SFX path: " .. AudioConfig.paths.sfx)
print("[AudioConfig]   Default music volume: " .. AudioConfig.volumes.musicDefault .. "%")
print("[AudioConfig]   Default SFX volume: " .. AudioConfig.volumes.sfxDefault .. "%")
