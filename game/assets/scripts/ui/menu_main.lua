-- ==========================================
-- R-Type Game - Main Menu UI
-- ==========================================

MainMenu = {
    title = {
        text = "R-TYPE",
        x = 960,  -- Centre horizontal
        y = 200,
        fontSize = 72,
        color = {255, 100, 0, 255}
    },
    
    subtitle = {
        text = "Epitech Project 2026",
        x = 960,
        y = 300,
        fontSize = 24,
        color = {200, 200, 200, 255}
    },
    
    buttons = {
        -- Solo Play
        {
            id = "btn_solo",
            text = "Solo Play",
            x = 760,
            y = 450,
            width = 400,
            height = 60,
            callback = "on_solo_play"
        },
        
        -- Multiplayer
        {
            id = "btn_multiplayer",
            text = "Multiplayer",
            x = 760,
            y = 540,
            width = 400,
            height = 60,
            callback = "on_multiplayer"
        },
        
        -- Settings
        {
            id = "btn_settings",
            text = "Settings",
            x = 760,
            y = 630,
            width = 400,
            height = 60,
            callback = "on_settings"
        },
        
        -- Quit
        {
            id = "btn_quit",
            text = "Quit",
            x = 760,
            y = 720,
            width = 400,
            height = 60,
            callback = "on_quit"
        }
    },
    
    background = {
        panel = {
            x = 660,
            y = 400,
            width = 600,
            height = 400
        }
    }
}

-- Callbacks functions (will be called by C++)
function on_solo_play()
    print("[LUA] Solo Play clicked!")
    -- Will push PlayState
end

function on_multiplayer()
    print("[LUA] Multiplayer clicked!")
    -- Will push MultiplayerMenuState
end

function on_settings()
    print("[LUA] Settings clicked!")
    -- Will push SettingsState
end

function on_quit()
    print("[LUA] Quit clicked!")
    -- Will close the game
end

print("[LUA] ✅ Main Menu UI loaded")

return MainMenu
