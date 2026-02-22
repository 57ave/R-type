-- ==========================================
-- R-Type Game - Host Game Menu
-- IP and port are read from game_config.lua (network section)
-- ==========================================

-- Load network defaults from game_config.lua
local _defaultPort = "12345"
local _ok, _cfg = pcall(function()
    return dofile("assets/scripts/config/game_config.lua")
end)
if _ok and _cfg and _cfg.network then
    _defaultPort = tostring(_cfg.network.server_port or 12345)
end

HostMenu = {
    title = {
        text = "HOST GAME",
        x = 960,
        y = 150,
        fontSize = 48,
        color = {255, 255, 255, 255}
    },
    
    -- Room name label
    room_label = {
        text = "Room Name:",
        x = 500,
        y = 350,
        fontSize = 24
    },
    
    -- Room name input
    room_input = {
        x = 750,
        y = 345,
        width = 400,
        height = 40,
        placeholder = "Enter room name...",
        maxLength = 32
    },
    
    -- Max players label
    players_label = {
        text = "Max Players:",
        x = 500,
        y = 450,
        fontSize = 24
    },
    
    -- Max players input
    players_input = {
        x = 750,
        y = 445,
        width = 400,
        height = 40,
        placeholder = "4",
        maxLength = 1
    },
    
    -- Port label
    port_label = {
        text = "Port:",
        x = 500,
        y = 550,
        fontSize = 24
    },
    
    -- Port input
    port_input = {
        x = 750,
        y = 545,
        width = 400,
        height = 40,
        placeholder = _defaultPort,
        maxLength = 5
    },
    
    buttons = {
        {
            id = "btn_start",
            text = "Start Server",
            x = 760,
            y = 650,
            width = 400,
            height = 60,
            callback = "on_start_server"
        },
        {
            id = "btn_cancel",
            text = "Cancel",
            x = 760,
            y = 730,
            width = 400,
            height = 60,
            callback = "on_cancel"
        }
    },
    
    background = {
        panel = {
            x = 460,
            y = 300,
            width = 1000,
            height = 500
        }
    }
}

function on_start_server()
    print("[LUA] Start Server clicked!")
end

function on_cancel()
    print("[LUA] Cancel clicked!")
end

print("[LUA] Host Menu UI loaded")

return HostMenu
