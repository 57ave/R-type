-- flappy_ui.lua: UI system for all game states
-- States: MENU, LOBBY, COUNTDOWN, PLAYING, GAMEOVER

local FlappyUI = {}

-- UI state
FlappyUI.currentState = "MENU"  -- MENU, LOBBY, COUNTDOWN, PLAYING, GAMEOVER
FlappyUI.entities = {}  -- All UI entities (text, buttons, etc.)
FlappyUI.isNetworked = false
FlappyUI.playerName = "Player"
FlappyUI.lobbyPlayers = {}  -- List of players in lobby
FlappyUI.leaderboard = {}  -- Final scores [{name, score, rank}, ...]
FlappyUI.localScore = 0
FlappyUI.countdown = 3

-- Fonts and text sizes
local FONT_LARGE = 72
local FONT_MEDIUM = 48
local FONT_SMALL = 32
local FONT_TINY = 20

-- Colors (RGBA hex)
local COLOR_WHITE = 0xFFFFFFFF
local COLOR_YELLOW = 0xFFFF00FF
local COLOR_CYAN = 0x00FFFFFF
local COLOR_GREEN = 0x00FF00FF
local COLOR_RED = 0xFF0000FF
local COLOR_ORANGE = 0xFFA500FF

-- Check if UI bindings are available
local function hasUIBindings()
    return UI ~= nil and UI.CreateText ~= nil
end

-- Helper to clear all UI entities
function FlappyUI.clearUI()
    for _, ent in ipairs(FlappyUI.entities) do
        if Flappy and Flappy.DestroyEntity then
            Flappy.DestroyEntity(ent)
        end
    end
    FlappyUI.entities = {}
end

-- Helper to create text entity using engine UI bindings
function FlappyUI.createText(x, y, text, fontSize, color)
    -- Try engine UI bindings first
    if hasUIBindings() then
        local entity = UI.CreateText({
            x = x,
            y = y,
            text = text,
            fontSize = fontSize or FONT_MEDIUM,
            color = color or COLOR_WHITE
        })
        if entity then
            table.insert(FlappyUI.entities, entity)
            return entity
        end
    end
    
    -- Fallback: just print to console
    print("[UI] Text: " .. text .. " at (" .. x .. ", " .. y .. ")")
    return nil
end

-- Helper to create button using engine UI bindings
function FlappyUI.createButton(x, y, width, height, text, callback)
    -- Try engine UI bindings first
    if hasUIBindings() and UI.CreateButton then
        local entity = UI.CreateButton({
            x = x,
            y = y,
            width = width,
            height = height,
            text = text,
            onClick = callback
        })
        if entity then
            table.insert(FlappyUI.entities, entity)
            return entity
        end
    end
    
    -- Fallback: just print to console
    print("[UI] Button: " .. text .. " at (" .. x .. ", " .. y .. ") callback=" .. callback)
    return nil
end

-- Show main menu (local/network choice)
function FlappyUI.showMenu()
    FlappyUI.clearUI()
    FlappyUI.currentState = "MENU"
    
    -- Title
    FlappyUI.createText(400, 100, "FLAPPY BIRD", FONT_LARGE, COLOR_YELLOW)
    FlappyUI.createText(400, 160, "BATTLE ROYALE", FONT_MEDIUM, COLOR_CYAN)
    
    -- Buttons
    FlappyUI.createButton(300, 300, 200, 60, "PLAY LOCAL", "OnPlayLocal")
    FlappyUI.createButton(300, 380, 200, 60, "MULTIPLAYER", "OnPlayMultiplayer")
    FlappyUI.createButton(300, 460, 200, 60, "QUIT", "OnQuit")
    
    -- Instructions
    FlappyUI.createText(400, 580, "Press SPACE to flap", FONT_SMALL, COLOR_WHITE)
    
    print("UI: Menu shown")
end

-- Show lobby (waiting for players)
function FlappyUI.showLobby()
    FlappyUI.clearUI()
    FlappyUI.currentState = "LOBBY"
    FlappyUI.isNetworked = true
    
    -- Title
    FlappyUI.createText(400, 80, "LOBBY", FONT_LARGE, COLOR_CYAN)
    
    -- Player list (will be updated)
    FlappyUI.createText(400, 180, "Waiting for players...", FONT_MEDIUM, COLOR_WHITE)
    
    -- Instructions
    FlappyUI.createText(400, 500, "Press R when ready", FONT_SMALL, COLOR_YELLOW)
    FlappyUI.createText(400, 540, "Waiting for all players to ready up", FONT_TINY, COLOR_WHITE)
    
    print("UI: Lobby shown")
end

-- Update lobby player list
function FlappyUI.updateLobby(players)
    FlappyUI.lobbyPlayers = players
    
    -- Clear old player list (keep first 5 UI elements: title, header, instructions x2, etc.)
    while #FlappyUI.entities > 5 do
        local ent = table.remove(FlappyUI.entities)
        if Flappy and Flappy.DestroyEntity then
            Flappy.DestroyEntity(ent)
        end
    end
    
    -- Show each player
    local y = 220
    for i, player in ipairs(players) do
        local status = player.ready and "[READY]" or "[NOT READY]"
        local color = player.ready and COLOR_GREEN or COLOR_RED
        local text = string.format("%d. %s %s", i, player.name, status)
        FlappyUI.createText(400, y, text, FONT_SMALL, color)
        y = y + 40
    end
    
    print("UI: Lobby updated with " .. #players .. " players")
end

-- Show countdown before game starts
function FlappyUI.showCountdown(count)
    FlappyUI.clearUI()
    FlappyUI.currentState = "COUNTDOWN"
    FlappyUI.countdown = count or 3
    
    -- Large countdown number
    local text = tostring(FlappyUI.countdown)
    if FlappyUI.countdown == 0 then
        text = "GO!"
    end
    FlappyUI.createText(400, 300, text, FONT_LARGE * 2, COLOR_YELLOW)
    
    print("UI: Countdown shown: " .. text)
end

-- Update countdown
function FlappyUI.updateCountdown(count)
    FlappyUI.countdown = count
    FlappyUI.showCountdown(count)
end

-- Show in-game UI (score)
function FlappyUI.showPlaying()
    FlappyUI.clearUI()
    FlappyUI.currentState = "PLAYING"
    
    -- Score display (top-center)
    FlappyUI.createText(400, 50, "Score: 0", FONT_LARGE, COLOR_WHITE)
    
    print("UI: Playing UI shown")
end

-- Update in-game score
function FlappyUI.updateScore(score)
    FlappyUI.localScore = score
    
    -- Update score text (first UI element in PLAYING state)
    if #FlappyUI.entities > 0 then
        if hasUIBindings() and UI.SetText then
            UI.SetText(FlappyUI.entities[1], "Score: " .. score)
        end
    end
    
    print("[UI] Score updated: " .. score)
end

-- Show game over screen with leaderboard
function FlappyUI.showGameOver(leaderboard)
    FlappyUI.clearUI()
    FlappyUI.currentState = "GAMEOVER"
    FlappyUI.leaderboard = leaderboard or {}
    
    -- Title
    FlappyUI.createText(400, 80, "GAME OVER", FONT_LARGE, COLOR_RED)
    
    -- Local score
    local yourScore = FlappyUI.localScore or 0
    FlappyUI.createText(400, 160, "Your Score: " .. yourScore, FONT_MEDIUM, COLOR_YELLOW)
    
    -- Leaderboard
    if #FlappyUI.leaderboard > 0 then
        FlappyUI.createText(400, 220, "LEADERBOARD", FONT_MEDIUM, COLOR_CYAN)
        
        local y = 280
        for i, entry in ipairs(FlappyUI.leaderboard) do
            local rank = entry.rank or i
            local name = entry.name or "Player"
            local score = entry.score or 0
            
            -- Color top 3 differently
            local color = COLOR_WHITE
            if rank == 1 then color = COLOR_YELLOW
            elseif rank == 2 then color = COLOR_ORANGE
            elseif rank == 3 then color = COLOR_CYAN
            end
            
            local text = string.format("%d. %s - %d", rank, name, score)
            FlappyUI.createText(400, y, text, FONT_SMALL, color)
            y = y + 35
            
            -- Only show top 10
            if i >= 10 then break end
        end
    end
    
    -- Return to menu button
    local btnY = #FlappyUI.leaderboard > 0 and 520 or 400
    FlappyUI.createButton(300, btnY, 200, 60, "MAIN MENU", "OnReturnToMenu")
    
    print("UI: Game Over shown with " .. #FlappyUI.leaderboard .. " entries")
end

-- Callback handlers (called from C++ via Lua state)
function OnPlayLocal()
    print("UI: Play Local clicked")
    if GameState and GameState.Start then
        GameState.Start("local")
    end
end

function OnPlayMultiplayer()
    print("UI: Play Multiplayer clicked")
    if GameState and GameState.Start then
        GameState.Start("network")
    end
end

function OnQuit()
    print("UI: Quit clicked")
    if GameState and GameState.Quit then
        GameState.Quit()
    end
end

function OnReturnToMenu()
    print("UI: Return to Menu clicked")
    FlappyUI.showMenu()
end

-- Initialize UI system
function FlappyUI.init()
    print("FlappyUI initialized")
    FlappyUI.showMenu()
end

-- Update UI (called each frame if needed)
function FlappyUI.update(dt)
    -- Update any animated UI elements here if needed
end

return FlappyUI
