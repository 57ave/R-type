-- ==========================================
-- R-Type Game - Multiplayer Menu UI
-- ==========================================

MultiplayerMenu = {
    title = {
        text = "MULTIPLAYER",
        x = 960,
        y = 150,
        fontSize = 48,
        color = {255, 255, 255, 255}
    },
    
    -- Main menu buttons (Host / Join / Back)
    main_buttons = {
        {
            id = "btn_host",
            text = "Host Game",
            x = 760,
            y = 400,
            width = 400,
            height = 60,
            callback = "on_host"
        },
        {
            id = "btn_join",
            text = "Join Game",
            x = 760,
            y = 500,
            width = 400,
            height = 60,
            callback = "on_join"
        },
        {
            id = "btn_back",
            text = "Back",
            x = 760,
            y = 600,
            width = 400,
            height = 60,
            callback = "on_back"
        }
    },
    
    background = {
        panel = {
            x = 660,
            y = 350,
            width = 600,
            height = 350
        }
    }
}

-- Callbacks
function on_host()
    print("[LUA] Host Game clicked!")
end

function on_join()
    print("[LUA] Join Game clicked!")
end

function on_back()
    print("[LUA] Back to main menu clicked!")
end

print("[LUA] Multiplayer Menu UI loaded")

return MultiplayerMenu
