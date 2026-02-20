-- ==========================================
-- R-Type Game - Lobby Menu (In-Room)
-- ==========================================

LobbyMenu = {
    title = {
        text = "LOBBY",
        x = 960,
        y = 100,
        fontSize = 48,
        color = {255, 255, 255, 255}
    },
    
    room_info = {
        text = "Room Name",  -- Will be set dynamically
        x = 960,
        y = 180,
        fontSize = 28,
        color = {200, 200, 200, 255}
    },
    
    -- Player list area
    player_list = {
        x = 500,
        y = 280,
        width = 920,
        height = 400,
        title = "Players:",
        title_fontSize = 24,
        item_height = 60,
        max_players = 4
    },
    
    buttons = {
        {
            id = "btn_ready",
            text = "Ready",
            x = 560,
            y = 820,
            width = 280,
            height = 60,
            callback = "on_toggle_ready"
        },
        {
            id = "btn_start",
            text = "Start Game",
            x = 860,
            y = 820,
            width = 280,
            height = 60,
            callback = "on_start_game"
        },
        {
            id = "btn_leave",
            text = "Leave Room",
            x = 1160,
            y = 820,
            width = 280,
            height = 60,
            callback = "on_leave_room"
        }
    },
    
    -- Chat area
    chat = {
        x = 500,
        y = 520,
        width = 920,
        height = 230,
        title = "Chat:",
        title_fontSize = 22,
        message_fontSize = 18,
        line_height = 22,
        max_visible = 8,
        input_y = 760,
        input_width = 920,
        input_height = 40
    },
    
    background = {
        panel = {
            x = 460,
            y = 250,
            width = 1000,
            height = 650
        }
    }
}

function on_toggle_ready()
    print("[LUA] Toggle Ready clicked!")
end

function on_start_game()
    print("[LUA] Start Game clicked!")
end

function on_leave_room()
    print("[LUA] Leave Room clicked!")
end

print("[LUA] ✅ Lobby Menu UI loaded")

return LobbyMenu
