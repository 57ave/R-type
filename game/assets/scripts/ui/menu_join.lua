-- ==========================================
-- R-Type Game - Join Game Menu (Server Browser)
-- ==========================================

JoinMenu = {
    title = {
        text = "JOIN GAME",
        x = 960,
        y = 150,
        fontSize = 48,
        color = {255, 255, 255, 255}
    },
    
    subtitle = {
        text = "Available Rooms:",
        x = 960,
        y = 250,
        fontSize = 24,
        color = {200, 200, 200, 255}
    },
    
    buttons = {
        {
            id = "btn_refresh",
            text = "Refresh",
            x = 500,
            y = 750,
            width = 300,
            height = 60,
            callback = "on_refresh"
        },
        {
            id = "btn_back",
            text = "Back",
            x = 1120,
            y = 750,
            width = 300,
            height = 60,
            callback = "on_back"
        }
    },
    
    background = {
        panel = {
            x = 400,
            y = 200,
            width = 1120,
            height = 650
        }
    },
    
    -- Room list area
    room_list = {
        x = 450,
        y = 320,
        width = 1020,
        height = 400,
        item_height = 80
    }
}

function on_refresh()
    print("[LUA] Refresh clicked!")
end

function on_back()
    print("[LUA] Back clicked!")
end

print("[LUA] ✅ Join Menu UI loaded")

return JoinMenu
