-- ==========================================
-- R-Type Game - Settings Menu UI
-- ==========================================

SettingsMenu = {
    title = {
        text = "SETTINGS",
        x = 960,
        y = 150,
        fontSize = 48,
        color = {255, 255, 255, 255}
    },
    
    -- Audio Settings Section
    audio = {
        section_title = {
            text = "Audio",
            x = 400,
            y = 300,
            fontSize = 32
        },
        
        -- Master Volume Slider
        master_volume = {
            label = "Master Volume",
            x = 400,
            y = 360,
            width = 500,
            height = 40,
            min = 0,
            max = 100,
            default = 100
        },
        
        -- Music Volume Slider
        music_volume = {
            label = "Music Volume",
            x = 400,
            y = 440,
            width = 500,
            height = 40,
            min = 0,
            max = 100,
            default = 70
        },
        
        -- SFX Volume Slider
        sfx_volume = {
            label = "SFX Volume",
            x = 400,
            y = 520,
            width = 500,
            height = 40,
            min = 0,
            max = 100,
            default = 80
        }
    },
    
    -- Graphics Settings Section
    graphics = {
        section_title = {
            text = "Graphics",
            x = 1100,
            y = 300,
            fontSize = 32
        },
        
        -- VSync Checkbox
        vsync = {
            label = "VSync",
            x = 1100,
            y = 360,
            default = true
        },
        
        -- Fullscreen Checkbox
        fullscreen = {
            label = "Fullscreen",
            x = 1100,
            y = 420,
            default = false
        },
        
        -- Show FPS Checkbox
        show_fps = {
            label = "Show FPS",
            x = 1100,
            y = 480,
            default = true
        }
    },
    
    -- Control Settings Section
    controls = {
        section_title = {
            text = "Controls",
            x = 400,
            y = 620,
            fontSize = 32
        },
        
        info_text = {
            text = "Press ESC to go back | Space to shoot | Arrows to move",
            x = 400,
            y = 680,
            fontSize = 18,
            color = {180, 180, 180, 255}
        }
    },
    
    -- Buttons
    buttons = {
        -- Apply
        {
            id = "btn_apply",
            text = "Apply",
            x = 660,
            y = 850,
            width = 250,
            height = 60,
            callback = "on_apply_settings"
        },
        
        -- Back
        {
            id = "btn_back",
            text = "Back",
            x = 1010,
            y = 850,
            width = 250,
            height = 60,
            callback = "on_back_to_menu"
        }
    },
    
    background = {
        panel = {
            x = 300,
            y = 200,
            width = 1320,
            height = 750
        }
    }
}

-- Callbacks
function on_apply_settings()
    print("[LUA] Apply settings clicked!")
    -- Save settings to Lua config
end

function on_back_to_menu()
    print("[LUA] Back to menu clicked!")
    -- Pop SettingsState, return to MainMenuState
end

print("[LUA]  Settings Menu UI loaded")

return SettingsMenu
