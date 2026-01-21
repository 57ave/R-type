--[[
    Simple UI for Flappy Bird
    Uses only rectangles and text - no sprites
]]--

local UI = {}

-- Colors (RGB)
local Colors = {
    White = {255, 255, 255},
    Black = {0, 0, 0},
    Gray = {128, 128, 128},
    DarkGray = {64, 64, 64},
    LightGray = {192, 192, 192},
    Red = {255, 0, 0},
    Green = {0, 255, 0},
    Blue = {0, 128, 255},
    Yellow = {255, 255, 0},
    Cyan = {0, 255, 255},
}

-- Current UI state
UI.currentScreen = "MENU"  -- MENU, COUNTDOWN, PLAYING, GAMEOVER
UI.countdownValue = 3
UI.scores = {}
UI.leaderboard = {}

-- Chat state
UI.chatInput = ""
UI.chatHistory = {}
UI.isTyping = false
UI.chatVisible = true
UI.maxChatLines = 8

function UI.init()
    print("✅ Simple UI initialized")
end

-- Main render function called from C++
function RenderUI()
    if not Flappy or not Flappy.DrawText then
        return  -- Drawing functions not available
    end

    if UI.currentScreen == "MENU" then
        UI.renderMenu()
    elseif UI.currentScreen == "COUNTDOWN" then
        UI.renderCountdown()
    elseif UI.currentScreen == "PLAYING" then
        UI.renderPlaying()
    elseif UI.currentScreen == "GAMEOVER" then
        UI.renderGameOver()
    elseif UI.currentScreen == "LOBBY" then
        UI.renderLobby()
    end
end

function UI.renderMenu()
    local centerX = 640
    local centerY = 360

    -- Title
    Flappy.DrawText("FLAPPY BIRD", centerX, 100, 60, 255, 255, 255)
    Flappy.DrawText("BATTLE ROYALE", centerX, 170, 40, 255, 255, 0)

    -- Menu buttons (just rectangles with text)
    -- PLAY LOCAL button
    Flappy.DrawRect(centerX - 150, 280, 300, 60, 0, 128, 255, 200)
    Flappy.DrawRectOutline(centerX - 150, 280, 300, 60, 255, 255, 255, 2)
    Flappy.DrawText("PLAY LOCAL [1]", centerX, 310, 30, 255, 255, 255)

    -- MULTIPLAYER button
    Flappy.DrawRect(centerX - 150, 360, 300, 60, 0, 128, 255, 200)
    Flappy.DrawRectOutline(centerX - 150, 360, 300, 60, 255, 255, 255, 2)
    Flappy.DrawText("MULTIPLAYER [2] [M]", centerX, 390, 30, 255, 255, 255)

    -- QUIT button
    Flappy.DrawRect(centerX - 150, 440, 300, 60, 255, 0, 0, 200)
    Flappy.DrawRectOutline(centerX - 150, 440, 300, 60, 255, 255, 255, 2)
    Flappy.DrawText("QUIT [3] [ESC]", centerX, 470, 30, 255, 255, 255)

    -- Instructions
    Flappy.DrawText("Press SPACE or 1 for local game", centerX, 560, 20, 192, 192, 192)
    Flappy.DrawText("Press M or 2 for multiplayer", centerX, 590, 20, 192, 192, 192)
    Flappy.DrawText("Press ESC or 3 to quit", centerX, 620, 20, 192, 192, 192)
end

function UI.renderCountdown()
    local centerX = 640
    local centerY = 360

    -- Semi-transparent overlay
    Flappy.DrawRect(0, 0, 1280, 720, 0, 0, 0, 128)

    -- Countdown text
    local text = tostring(UI.countdownValue)
    if UI.countdownValue == 0 then
        text = "GO!"
    end
    
    Flappy.DrawText(text, centerX, centerY, 120, 255, 255, 0)
end

function UI.renderPlaying()
    -- Score display (top center)
    local centerX = 640
    
    Flappy.DrawText("Score: " .. (Game.scores[Game.localPlayerId] or 0), centerX, 40, 40, 255, 255, 255)
    
    -- Instructions (bottom)
    Flappy.DrawText("SPACE to flap", centerX, 690, 18, 192, 192, 192)
end

function UI.renderGameOver()
    print("[DEBUG] renderGameOver called!")
    local centerX = 640
    local centerY = 360

    -- Semi-transparent overlay
    Flappy.DrawRect(0, 0, 1280, 720, 0, 0, 0, 180)
    print("[DEBUG] Overlay drawn")

    -- Title
    Flappy.DrawText("GAME OVER", centerX, 100, 70, 255, 0, 0)
    print("[DEBUG] Title drawn")

    -- Score
    local playerScore = 0
    if Game and Game.scores and Game.localPlayerId then
        playerScore = Game.scores[Game.localPlayerId] or 0
    end
    print("[DEBUG] Player score: " .. playerScore)
    Flappy.DrawText("Your Score: " .. playerScore, centerX, 200, 40, 255, 255, 255)
    
    -- Check if player won (30 points)
    if playerScore >= 30 then
        Flappy.DrawText("YOU WIN!", centerX, 260, 50, 255, 255, 0)
    end

    -- Leaderboard
    Flappy.DrawText("LEADERBOARD", centerX, 320, 35, 255, 255, 0)
    
    if UI.leaderboard and #UI.leaderboard > 0 then
        for i, entry in ipairs(UI.leaderboard) do
            if i <= 5 then  -- Show top 5
                local y = 360 + (i - 1) * 40
                local text = string.format("%d. %s - %d", entry.rank, entry.name, entry.score)
                local color = {255, 255, 255}
                
                -- Highlight local player
                if entry.playerId == Game.localPlayerId then
                    color = {255, 255, 0}  -- Yellow for local player
                end
                
                Flappy.DrawText(text, centerX, y, 28, color[1], color[2], color[3])
            end
        end
    end

    -- Instructions
    Flappy.DrawText("Press SPACE or ESC to return to menu", centerX, 640, 22, 192, 192, 192)
end

function UI.renderLobby()
    local centerX = 640
    local centerY = 360

    -- Title
    Flappy.DrawText("MULTIPLAYER LOBBY", centerX, 100, 50, 255, 255, 0)

    -- Info
    Flappy.DrawText("Waiting for players...", centerX, 250, 30, 255, 255, 255)
    
    -- Player count (you'll need to update this from network data)
    local playerCount = 1  -- TODO: Get from server
    local minPlayers = 2
    Flappy.DrawText("Players: " .. playerCount .. " / 4 (Min: " .. minPlayers .. ")", centerX, 320, 28, 192, 192, 192)

    -- Instructions
    Flappy.DrawText("Waiting for more players to join...", centerX, 450, 22, 128, 128, 128)
    Flappy.DrawText("Press ESC to return to menu", centerX, 640, 20, 192, 192, 192)

    -- Chat Rendering
    if UI.chatVisible then
        local chatX = 50
        local chatY = 450
        local lineHeight = 25
        
        -- Draw Chat Box Background
        Flappy.DrawRect(chatX - 10, chatY - 10, 400, 220, 0, 0, 0, 150)
        Flappy.DrawRectOutline(chatX - 10, chatY - 10, 400, 220, 100, 100, 100, 1)

        -- Draw History (Last N messages)
        local historyCount = #UI.chatHistory
        local startIdx = math.max(1, historyCount - UI.maxChatLines + 1)
        
        for i = startIdx, historyCount do
            local msg = UI.chatHistory[i]
            local y = chatY + (i - startIdx) * lineHeight
            local displayText = msg.sender .. ": " .. msg.message
            Flappy.DrawTextLeft(displayText, chatX, y, 18, 200, 200, 200)
        end
        
        -- Draw Input Line
        local inputY = chatY + (UI.maxChatLines * lineHeight) + 10
        Flappy.DrawRect(chatX - 5, inputY - 5, 390, 30, 50, 50, 50, 200)
        
        local inputText = UI.chatInput
        if UI.isTyping then
            inputText = inputText .. "_"
        end
        Flappy.DrawTextLeft("Chat: " .. inputText, chatX, inputY, 18, 255, 255, 255)
    end
end

function UI.addChatMessage(sender, message)
    table.insert(UI.chatHistory, {sender = sender, message = message})
    if #UI.chatHistory > 20 then
        table.remove(UI.chatHistory, 1)
    end
end

function UI.onTextEntered(char)
    if not UI.isTyping then return end
    
    -- basic filtering (only printable)
    local byte = string.byte(char)
    if byte >= 32 and byte <= 126 then
        if #UI.chatInput < 40 then
            UI.chatInput = UI.chatInput .. char
        end
    end
end

function UI.onKeyPressed(keyCode)
    -- Toggle chat typing with Enter
    -- Key code 58 is Enter (sf::Keyboard::Enter) - wait, need to verify
    -- Assuming generic mapping or passing raw int. SFML Enter is 58.
    -- Backspace is 59.
    
    -- Let's use generic numbers for now, assuming standard SFML mappings
    local Key_Enter = 58
    local Key_Backspace = 59
    local Key_T = 19
    local Key_Esc = 36
    
    if keyCode == Key_Enter then
        if UI.isTyping then
            -- Send message
            if UI.chatInput ~= "" then
                if Network and Network.sendChat then
                    print("[UI] Sending chat: " .. UI.chatInput)
                    Network.sendChat(UI.chatInput)
                end
                UI.chatInput = ""
            end
            UI.isTyping = false
        else
            UI.isTyping = true
            -- Consume input so it doesn't type 't' or whatever
        end
    elseif keyCode == Key_Backspace then
        if UI.isTyping and #UI.chatInput > 0 then
            UI.chatInput = string.sub(UI.chatInput, 1, -2)
        end
    elseif keyCode == Key_Esc then
        if UI.isTyping then
            UI.isTyping = false
        end
    elseif keyCode == Key_T and not UI.isTyping and UI.currentScreen == "LOBBY" then
        -- Optional: Press T to start typing
        UI.isTyping = true
    end
end

-- Functions to update UI state (called from game logic)
function UI.showMenu()
    UI.currentScreen = "MENU"
    print("[SimpleUI] Showing menu")
end

function UI.showCountdown(value)
    UI.currentScreen = "COUNTDOWN"
    UI.countdownValue = value
    print("[SimpleUI] Countdown: " .. value)
end

function UI.updateCountdown(value)
    UI.countdownValue = value
end

function UI.showPlaying()
    UI.currentScreen = "PLAYING"
    print("[SimpleUI] Showing playing UI")
end

function UI.showLobby()
    UI.currentScreen = "LOBBY"
    print("[SimpleUI] Showing lobby UI")
end

function UI.showGameOver(leaderboard)
    UI.currentScreen = "GAMEOVER"
    UI.leaderboard = leaderboard or {}
    print("[SimpleUI] Showing game over with " .. #UI.leaderboard .. " entries")
end

function UI.update(dt)
    -- Nothing to update for simple UI
end

return UI
