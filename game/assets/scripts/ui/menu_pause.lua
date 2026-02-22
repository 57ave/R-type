-- ==========================================
-- R-Type Game - Pause Menu UI
-- ==========================================

PauseMenu = {
    title = {
        text = "PAUSED",
        x = 960,
        y = 300,
        fontSize = 64,
        color = {255, 255, 0, 255}
    },
    
    buttons = {
        -- Resume
        {
            id = "btn_resume",
            text = "Resume",
            x = 760,
            y = 450,
            width = 400,
            height = 60,
            callback = "on_resume"
        },
        
        -- Settings
        {
            id = "btn_settings",
            text = "Settings",
            x = 760,
            y = 540,
            width = 400,
            height = 60,
            callback = "on_settings"
        },
        
        -- Quit to Menu
        {
            id = "btn_quit_menu",
            text = "Quit to Menu",
            x = 760,
            y = 630,
            width = 400,
            height = 60,
            callback = "on_quit_to_menu"
        }
    },
    
    background = {
        overlay = {
            x = 0,
            y = 0,
            width = 1920,
            height = 1080,
            color = {0, 0, 0, 180}  -- Semi-transparent black
        },
        
        panel = {
            x = 660,
            y = 250,
            width = 600,
            height = 500
        }
    }
}

-- Callbacks
function on_resume()
    print("[LUA] Resume clicked!")
    -- Pop PauseState, return to PlayState
end

function on_settings()
    print("[LUA] Settings from pause clicked!")
    -- Push SettingsState
end

function on_quit_to_menu()
    print("[LUA] Quit to menu clicked!")
    -- Pop all states, push MainMenuState
end

print("[LUA]  Pause Menu UI loaded")

return PauseMenu
