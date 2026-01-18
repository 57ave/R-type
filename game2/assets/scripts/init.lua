--[[
    Flappy Bird - Battle Royale
    Main initialization script
    
    This is the entry point for all Lua game logic.
    The C++ side only handles:
    - ECS initialization
    - Window/rendering
    - Input event forwarding
    
    ALL game logic is here in Lua!
]]--

print("🐦 [Lua] Flappy Bird scripts loading...")

-- Get script base path
local scriptPath = "game2/assets/scripts/"

-- Helper for safe module loading
local function safeRequire(modName)
    local success, result = pcall(require, modName)
    if not success then
        print("WARNING: Could not load module '" .. modName .. "': " .. tostring(result))
        return nil
    end
    return result
end

-- Load modules (may fail if paths differ)
local FlappyUI = safeRequire("ui_simple") or {}  -- Use simple UI
-- NOTE: Network is provided by C++ bindings, not a Lua module

-- ============================================
-- GLOBAL GAME STATE
-- ============================================
GameState = {
    MENU = "MENU",
    WAITING = "WAITING",     -- Waiting for opponent (network only)
    COUNTDOWN = "COUNTDOWN", -- 3, 2, 1, GO!
    PLAYING = "PLAYING",
    GAMEOVER = "GAMEOVER"
}

-- Make FlappyUI global
UI = FlappyUI
-- Network is provided by C++ bindings (see FlappyGame.cpp)

Game = {
    state = GameState.MENU,
    mode = "local",  -- "local" or "network"
    countdown = 3,
    countdownTimer = 0,
    
    -- Screen dimensions (set from C++)
    screenWidth = SCREEN_WIDTH or 1280,
    screenHeight = SCREEN_HEIGHT or 720,
    
    -- Entities
    birds = {},      -- { [playerId] = entityId }
    pipes = {},      -- { entityId, ... }
    background = nil,
    
    -- Scores
    scores = {},     -- { [playerId] = score }
    
    -- Network state (simplified!)
    localPlayerId = 1,
    opponentPlayerId = 2,
    isConnectedToServer = false,
    waitingForOpponent = false,
    
    -- Physics constants
    gravity = 980,           -- pixels/s²
    flapStrength = 350,      -- pixels/s (upward impulse)
    terminalVelocity = 600,  -- max fall speed
    
    -- Pipe settings
    pipeSpeed = 200,         -- pixels/s
    pipeSpawnInterval = 2.0, -- seconds
    pipeGapHeight = 180,     -- pixels (space to fly through)
    pipeWidth = 80,
    pipeSpawnTimer = 0,
    
    -- Bird settings
    birdSize = 34,           -- pixels
}

-- ============================================
-- COMPONENT STORAGE (Lua-side extensions)
-- ============================================
-- These extend the C++ ECS components with game-specific data
Components = {
    FlappyBird = {},  -- { [entityId] = { playerId, isAlive, color, flapStrength } }
    Gravity = {},     -- { [entityId] = { force, terminalVelocity } }
    Pipe = {},        -- { [entityId] = { gapY, gapHeight, passed, isTop } }
    Score = {},       -- { [entityId] = { value, playerId } }
}

-- ============================================
-- INITIALIZATION
-- ============================================
function Init()
    print("🐦 [Lua] Init() called")
    print("   Screen: " .. Game.screenWidth .. "x" .. Game.screenHeight)
    
    -- Initialize UI system (with nil check)
    if UI and UI.init then
        UI.init()
    else
        print("WARNING: UI system not available")
    end
    
    -- Network is provided by C++ bindings, not a Lua module
    -- No initialization needed here
    
    -- Start at menu
    SetGameState(GameState.MENU)
    
    print("🐦 [Lua] Init() complete!")
    return true
end

-- ============================================
-- GAME STATE MANAGEMENT
-- ============================================
function SetGameState(newState)
    local oldState = Game.state
    Game.state = newState
    
    print("🎮 [GameState] " .. oldState .. " -> " .. newState)
    
    if newState == GameState.MENU then
        OnEnterMenu()
    elseif newState == GameState.WAITING then
        OnEnterWaiting()
    elseif newState == GameState.COUNTDOWN then
        OnEnterCountdown()
    elseif newState == GameState.PLAYING then
        OnEnterPlaying()
    elseif newState == GameState.GAMEOVER then
        OnEnterGameOver()
    end
end

function OnEnterMenu()
    -- Clean up any existing entities
    CleanupGame()
    
    -- Create background
    CreateBackground()
    
    -- Show menu UI (with nil check)
    if UI and UI.showMenu then
        UI.showMenu()
    end
    
    print("📋 [Menu] Ready!")
end

function OnEnterWaiting()
    print("⏳ [Waiting] Waiting for opponent...")
    Game.waitingForOpponent = true
    
    -- Clean up any existing entities
    CleanupGame()
    
    -- Create background
    CreateBackground()
    
    -- Show waiting UI (with nil check)
    if UI and UI.showWaiting then
        UI.showWaiting()
    else
        print("   (UI.showWaiting not available)")
    end
end

function OnEnterCountdown()
    Game.countdown = 3
    Game.countdownTimer = 0
    
    -- Create birds (both players in network mode!)
    if Game.mode == "network" then
        -- Create both birds (local player + opponent)
        CreateBird(1, Game.screenWidth / 4, Game.screenHeight / 2)
        CreateBird(2, Game.screenWidth / 4, Game.screenHeight / 2 + 50)  -- Slightly offset
    else
        -- Local mode: only create local player bird
        CreateBird(Game.localPlayerId, Game.screenWidth / 4, Game.screenHeight / 2)
    end
    
    -- Show countdown UI (with nil check)
    if UI and UI.showCountdown then
        UI.showCountdown(Game.countdown)
    end
    
    print("⏱️ [Countdown] 3...")
end

function OnEnterPlaying()
    Game.pipeSpawnTimer = 0
    
    -- Show playing UI (score) (with nil check)
    if UI and UI.showPlaying then
        UI.showPlaying()
    end
    
    print("🎮 [Playing] GO!")
end

function OnEnterGameOver()
    local winnerId = GetWinner()
    local winnerScore = Game.scores[winnerId] or 0
    print("🏆 [GameOver] Winner: Player " .. winnerId .. " with score: " .. winnerScore)
    
    -- Build leaderboard
    local leaderboard = {}
    for playerId, score in pairs(Game.scores) do
        table.insert(leaderboard, {
            playerId = playerId,
            name = "Player " .. playerId,
            score = score
        })
        print("   Player " .. playerId .. ": " .. score .. " points")
    end
    
    -- Sort by score descending
    table.sort(leaderboard, function(a, b) return a.score > b.score end)
    
    -- Add ranks
    for i, entry in ipairs(leaderboard) do
        entry.rank = i
    end
    
    -- Show game over UI (with nil check)
    if UI and UI.showGameOver then
        print("📺 Calling UI.showGameOver with " .. #leaderboard .. " entries")
        UI.showGameOver(leaderboard)
    else
        print("⚠️ WARNING: UI.showGameOver not available!")
    end
    
    print("🎮 Game Over state entered successfully")
end

-- ============================================
-- ENTITY CREATION
-- ============================================
function CreateBackground()
    if Game.background then
        Flappy.DestroyEntity(Game.background)
    end
    
    local entity = Flappy.CreateEntity()
    Game.background = entity
    
    -- Add Transform component for Lua (via C++ Coordinator)
    Coordinator.AddTransform(entity, Transform.new(0, 0, 0))
    
    -- Add Position for RenderSystem (via C++ Flappy helper)
    Flappy.SetPosition(entity, 0, 0)
    
    -- Setup sprite using C++ helper (handles texture + component creation)
    local success = Flappy.SetupSprite(entity, Flappy.Textures.Background, 1280, 720, 0)
    if not success then
        print("WARNING: Failed to setup background sprite")
    end
    
    print("🖼️ Created background entity: " .. entity)
    return entity
end

function CreateBird(playerId, x, y)
    local entity = Flappy.CreateEntity()
    
    -- Store in birds table
    Game.birds[playerId] = entity
    
    -- Initialize score
    Game.scores[playerId] = 0
    
    -- Add Transform for Lua
    Coordinator.AddTransform(entity, Transform.new(x, y, 0))
    
    -- Add Position for RenderSystem
    Flappy.SetPosition(entity, x, y)
    
    -- Add Velocity (both ECS and components versions)
    Coordinator.AddVelocity(entity, Velocity.new(0, 0, Game.terminalVelocity))
    Flappy.SetVelocity(entity, 0, 0)  -- Add ::Velocity for MovementSystem
    
    -- Add Tag (ECS Tag for Lua)
    Coordinator.AddTag(entity, Tag.new("Bird"))
    
    -- Setup sprite using C++ helper
    local success = Flappy.SetupSprite(entity, Flappy.Textures.Bird, Game.birdSize, Game.birdSize, 10)
    if not success then
        print("WARNING: Failed to setup bird sprite")
    end
    
    -- Note: Collision detection is handled in Lua (see CheckCollisions)
    -- We store the hitbox size in Components.FlappyBird
    
    -- Store Lua-side component data
    Components.FlappyBird[entity] = {
        playerId = playerId,
        isAlive = true,
        color = playerId,  -- Different color per player
        flapStrength = Game.flapStrength,
        hitboxSize = Game.birdSize  -- For collision detection
    }
    
    Components.Gravity[entity] = {
        force = Game.gravity,
        terminalVelocity = Game.terminalVelocity
    }
    
    print("🐦 Created bird for player " .. playerId .. " at (" .. x .. ", " .. y .. ") entity: " .. entity)
    return entity
end

function CreatePipePair(gapY)
    local gapHeight = Game.pipeGapHeight
    local pipeWidth = Game.pipeWidth
    local screenHeight = Game.screenHeight
    local spawnX = Game.screenWidth + pipeWidth
    
    -- Top pipe (from top of screen to gap)
    local topPipeHeight = math.max(1, gapY)  -- Ensure at least 1 pixel height
    local topEntity = Flappy.CreateEntity()
    
    Coordinator.AddTransform(topEntity, Transform.new(spawnX, 0, 0))
    Coordinator.AddVelocity(topEntity, Velocity.new(-Game.pipeSpeed, 0, 0))
    Coordinator.AddTag(topEntity, Tag.new("Pipe"))
    -- Note: Collision detection is handled in Lua
    
    -- Setup sprite using C++ helper
    Flappy.SetupSprite(topEntity, Flappy.Textures.Pipe, pipeWidth, topPipeHeight, 5)
    
    Components.Pipe[topEntity] = {
        gapY = gapY,
        gapHeight = gapHeight,
        passed = false,
        isTop = true,
        width = pipeWidth,
        height = topPipeHeight
    }
    
    table.insert(Game.pipes, topEntity)
    
    -- Bottom pipe (from gap bottom to screen bottom)
    local bottomY = gapY + gapHeight
    local bottomPipeHeight = math.max(1, screenHeight - bottomY)
    local bottomEntity = Flappy.CreateEntity()
    
    Coordinator.AddTransform(bottomEntity, Transform.new(spawnX, bottomY, 0))
    Coordinator.AddVelocity(bottomEntity, Velocity.new(-Game.pipeSpeed, 0, 0))
    Coordinator.AddTag(bottomEntity, Tag.new("Pipe"))
    -- Note: Collision detection is handled in Lua
    
    -- Setup sprite using C++ helper
    Flappy.SetupSprite(bottomEntity, Flappy.Textures.Pipe, pipeWidth, bottomPipeHeight, 5)
    
    Components.Pipe[bottomEntity] = {
        gapY = gapY,
        gapHeight = gapHeight,
        passed = false,
        isTop = false,
        width = pipeWidth,
        height = bottomPipeHeight
    }
    
    table.insert(Game.pipes, bottomEntity)
    
    print("🚧 Created pipe pair at gapY=" .. gapY .. " (entities: " .. topEntity .. ", " .. bottomEntity .. ")")
    return topEntity, bottomEntity
end

-- ============================================
-- NETWORK HELPERS
-- ============================================
function UpdateBirdFromServer(playerId, x, y, velY, isAlive)
    local entity = Game.birds[playerId]
    if not entity then
        print("⚠️ [Network] No bird entity for player " .. playerId)
        return
    end
    
    -- Update position
    if Coordinator.HasTransform(entity) then
        local pos = Coordinator.GetTransform(entity)
        pos.x = x
        pos.y = y
        Flappy.SetPosition(entity, x, y)
    end
    
    -- Update velocity
    if Coordinator.HasVelocity(entity) then
        local vel = Coordinator.GetVelocity(entity)
        vel.dy = velY
        Flappy.SetVelocity(entity, 0, velY)
    end
    
    -- Update alive state
    local birdData = Components.FlappyBird[entity]
    if birdData then
        birdData.isAlive = isAlive
    end
end

function CreatePipeFromServer(x, gapY)
    local gapHeight = Game.pipeGapHeight
    local pipeWidth = Game.pipeWidth
    local screenHeight = Game.screenHeight
    
    -- Top pipe
    local topPipeHeight = math.max(1, gapY)
    local topEntity = Flappy.CreateEntity()
    
    Coordinator.AddTransform(topEntity, Transform.new(x, 0, 0))
    Coordinator.AddVelocity(topEntity, Velocity.new(-Game.pipeSpeed, 0, 0))
    Coordinator.AddTag(topEntity, Tag.new("Pipe"))
    
    Flappy.SetupSprite(topEntity, Flappy.Textures.Pipe, pipeWidth, topPipeHeight, 5)
    
    Components.Pipe[topEntity] = {
        gapY = gapY,
        gapHeight = gapHeight,
        passed = false,
        isTop = true,
        width = pipeWidth,
        height = topPipeHeight
    }
    
    table.insert(Game.pipes, topEntity)
    
    -- Bottom pipe
    local bottomY = gapY + gapHeight
    local bottomPipeHeight = math.max(1, screenHeight - bottomY)
    local bottomEntity = Flappy.CreateEntity()
    
    Coordinator.AddTransform(bottomEntity, Transform.new(x, bottomY, 0))
    Coordinator.AddVelocity(bottomEntity, Velocity.new(-Game.pipeSpeed, 0, 0))
    Coordinator.AddTag(bottomEntity, Tag.new("Pipe"))
    
    Flappy.SetupSprite(bottomEntity, Flappy.Textures.Pipe, pipeWidth, bottomPipeHeight, 5)
    
    Components.Pipe[bottomEntity] = {
        gapY = gapY,
        gapHeight = gapHeight,
        passed = false,
        isTop = false,
        width = pipeWidth,
        height = bottomPipeHeight
    }
    
    table.insert(Game.pipes, bottomEntity)
    
    print("🎋 [Network] Created pipe from server at x=" .. x .. ", gapY=" .. gapY)
    return topEntity, bottomEntity
end

-- ============================================
-- GAME LOGIC
-- ============================================
function Flap(playerId)
    local entity = Game.birds[playerId]
    if not entity then return end
    
    local birdData = Components.FlappyBird[entity]
    if not birdData or not birdData.isAlive then return end
    
    -- Apply upward impulse
    if Coordinator.HasVelocity(entity) then
        local vel = Coordinator.GetVelocity(entity)
        vel.dy = -birdData.flapStrength
        -- Sync to ::Velocity
        Flappy.SetVelocity(entity, vel.dx, vel.dy)
        print("🐦 Player " .. playerId .. " flapped! vel.dy = " .. vel.dy)
    end
end

function KillBird(playerId)
    local entity = Game.birds[playerId]
    if not entity then return end
    
    local birdData = Components.FlappyBird[entity]
    if birdData then
        if birdData.isAlive then
            birdData.isAlive = false
            local finalScore = Game.scores[playerId] or 0
            print("☠️ Player " .. playerId .. " died! Final score: " .. finalScore)
            
            -- Check if game over
            CheckGameOver()
        end
    end
end

-- ============================================
-- NETWORK SYNCHRONIZATION HELPERS
-- ============================================
function UpdateBirdFromServer(playerId, x, y, velY, isAlive)
    local entity = Game.birds[playerId]
    if not entity then
        -- Bird doesn't exist yet, create it
        CreateBird(playerId, x, y)
        return
    end
    
    -- Update transform from server (authoritative!)
    if Coordinator.HasTransform(entity) then
        local transform = Coordinator.GetTransform(entity)
        transform.x = x
        transform.y = y
    end
    
    -- Update velocity
    if Coordinator.HasVelocity(entity) then
        local velocity = Coordinator.GetVelocity(entity)
        velocity.dy = velY
    end
    
    -- Update alive status
    local birdData = Components.FlappyBird[entity]
    if birdData then
        birdData.isAlive = isAlive
    end
end

function CreatePipeFromServer(x, gapY)
    local gapHeight = Game.pipeGapHeight
    local pipeWidth = Game.pipeWidth
    local screenHeight = Game.screenHeight
    
    -- Top pipe (from top of screen to gapY - gapHeight/2)
    local topPipeHeight = gapY - (gapHeight / 2)
    local topEntity = Flappy.CreateEntity()
    
    Coordinator.AddTransform(topEntity, Transform.new(x, 0, 0))
    Flappy.SetPosition(topEntity, x, 0)
    Flappy.SetupSprite(topEntity, Flappy.Textures.PipeGreen, Game.pipeWidth, topPipeHeight, 0)
    
    Components.Pipe[topEntity] = {
        gapY = gapY,
        gapHeight = gapHeight,
        passed = false,
        isTop = true
    }
    
    table.insert(Game.pipes, topEntity)
    
    -- Bottom pipe (from gapY + gapHeight/2 to bottom of screen)
    local bottomPipeY = gapY + (gapHeight / 2)
    local bottomPipeHeight = Game.screenHeight - bottomPipeY
    local bottomEntity = Flappy.CreateEntity()
    
    Coordinator.AddTransform(bottomEntity, Transform.new(x, bottomPipeY, 0))
    Flappy.SetPosition(bottomEntity, x, bottomPipeY)
    Flappy.SetupSprite(bottomEntity, Flappy.Textures.PipeGreen, Game.pipeWidth, bottomPipeHeight, 0)
    
    Components.Pipe[bottomEntity] = {
        gapY = gapY,
        gapHeight = gapHeight,
        passed = false,
        isTop = false
    }
    
    table.insert(Game.pipes, bottomEntity)
    
    print("🎋 Created pipe pair at x=" .. x .. ", gapY=" .. gapY)
end

function CheckGameOver()
    local alivePlayers = 0
    local lastAlive = nil
    
    for playerId, entity in pairs(Game.birds) do
        local birdData = Components.FlappyBird[entity]
        if birdData and birdData.isAlive then
            alivePlayers = alivePlayers + 1
            lastAlive = playerId
        end
    end
    
    print("🔍 CheckGameOver: " .. alivePlayers .. " alive, mode=" .. Game.mode)
    
    if alivePlayers == 0 then
        -- Everyone dead
        print("💀 All players dead - triggering game over")
        SetGameState(GameState.GAMEOVER)
    elseif alivePlayers == 1 and Game.mode == "network" then
        -- Battle Royale: last one standing wins
        print("👑 Last player standing (Player " .. lastAlive .. ") - triggering game over")
        SetGameState(GameState.GAMEOVER)
    end
end

function GetWinner()
    local highestScore = -1
    local winnerId = 1
    
    for playerId, score in pairs(Game.scores) do
        if score > highestScore then
            highestScore = score
            winnerId = playerId
        end
    end
    
    return winnerId
end

function CleanupGame()
    -- Destroy all birds
    for _, entity in pairs(Game.birds) do
        Flappy.DestroyEntity(entity)
        Components.FlappyBird[entity] = nil
        Components.Gravity[entity] = nil
    end
    Game.birds = {}
    
    -- Destroy all pipes
    for _, entity in ipairs(Game.pipes) do
        Flappy.DestroyEntity(entity)
        Components.Pipe[entity] = nil
    end
    Game.pipes = {}
    
    -- Reset scores
    Game.scores = {}
    
    print("🧹 Game cleaned up")
end

-- ============================================
-- UPDATE LOOP (called every frame from C++)
-- ============================================
function Update(deltaTime)
    -- Update network (with nil check)
    if Game.mode == "network" and Network and Network.update then
        Network.update(deltaTime)
    end
    
    -- Update UI (with nil check)
    if UI and UI.update then
        UI.update(deltaTime)
    end
    
    -- Update game state
    if Game.state == GameState.MENU then
        UpdateMenu(deltaTime)
    elseif Game.state == GameState.WAITING then
        UpdateWaiting(deltaTime)
    elseif Game.state == GameState.COUNTDOWN then
        UpdateCountdown(deltaTime)
    elseif Game.state == GameState.PLAYING then
        UpdatePlaying(deltaTime)
    elseif Game.state == GameState.GAMEOVER then
        UpdateGameOver(deltaTime)
    end
end

function UpdateMenu(deltaTime)
    -- Handle keyboard shortcuts for menu navigation
    if Flappy and Flappy.Input then
        -- SPACE or 1 = Play Local
        if Flappy.Input.SpaceJustPressed or Flappy.Input.Key1JustPressed then
            OnPlayLocal()
        end
        
        -- M or 2 = Play Multiplayer
        if Flappy.Input.MJustPressed or Flappy.Input.Key2JustPressed then
            OnPlayMultiplayer()
        end
        
        -- ESC or 3 = Quit
        if Flappy.Input.EscapeJustPressed or Flappy.Input.Key3JustPressed then
            OnQuit()
        end
    end
end

function UpdateWaiting(deltaTime)
    -- Update network to receive packets (GAME_START, etc.)
    if Game.mode == "network" and Network and Network.update then
        Network.update(deltaTime)
        
        -- Process received packets
        if Network.hasPackets then
            while Network.hasPackets() do
                local packet = Network.getNextPacket()
                if packet then
                    HandleNetworkPacket(packet)
                end
            end
        end
    end
    
    -- Handle input in waiting screen
    if Flappy and Flappy.Input then
        -- ESC = Return to menu and disconnect
        if Flappy.Input.EscapeJustPressed then
            print("🔙 Returning to menu from waiting...")
            OnReturnToMenu()
        end
    end
end

function UpdateCountdown(deltaTime)
    Game.countdownTimer = Game.countdownTimer + deltaTime
    
    if Game.countdownTimer >= 1.0 then
        Game.countdownTimer = 0
        Game.countdown = Game.countdown - 1
        
        if Game.countdown > 0 then
            print("⏱️ [Countdown] " .. Game.countdown .. "...")
            if UI and UI.updateCountdown then
                UI.updateCountdown(Game.countdown)
            end
        else
            if UI and UI.updateCountdown then
                UI.updateCountdown(0)  -- "GO!"
            end
            -- Wait a brief moment before starting
            -- In a real implementation, use a timer
            SetGameState(GameState.PLAYING)
        end
    end
end

function UpdatePlaying(deltaTime)
    -- Handle flap input
    if Game.mode == "local" and Flappy and Flappy.Input and Flappy.Input.SpaceJustPressed then
        Flap(Game.localPlayerId)
    elseif Game.mode == "network" and Flappy and Flappy.Input and Flappy.Input.SpaceJustPressed then
        -- In network mode, ONLY send input to server (no local prediction)
        if Network and Network.sendFlap then
            print("⬆️ [Input] Sending flap to server")
            Network.sendFlap(Game.localPlayerId)
        end
    end
    
    -- Update network (receive packets from server)
    if Game.mode == "network" and Network and Network.update then
        Network.update(deltaTime)
        
        -- Process received packets
        if Network.hasPackets then
            while Network.hasPackets() do
                local packet = Network.getNextPacket()
                if packet then
                    HandleNetworkPacket(packet)
                end
            end
        end
    end
    
    -- ======= LOCAL MODE ONLY: Run physics locally =======
    if Game.mode == "local" then
        -- Update gravity for all birds
        UpdateGravity(deltaTime)
        
        -- Update movement (positions from velocities)
        UpdateMovement(deltaTime)
        
        -- Spawn pipes
        UpdatePipeSpawning(deltaTime)
        
        -- Check collisions
        UpdateCollisions()
        
        -- Update scoring
        UpdateScoring()
        
        -- Clean up off-screen pipes
        CleanupPipes()
        
        -- Check bounds (floor/ceiling death)
        CheckBounds()
    end
    -- ======= NETWORK MODE: Server is authoritative =======
    -- All physics/collision/scoring is done server-side
    -- We only render what the server tells us!
end

function UpdateGameOver(deltaTime)
    -- Handle space or ESC to return to menu
    if Flappy and Flappy.Input then
        if Flappy.Input.SpaceJustPressed or Flappy.Input.EscapeJustPressed then
            print("🔙 Returning to menu from game over...")
            OnReturnToMenu()
        end
    end
end

-- ============================================
-- PHYSICS SYSTEMS (Lua)
-- ============================================
function UpdateGravity(deltaTime)
    for entity, gravData in pairs(Components.Gravity) do
        local birdData = Components.FlappyBird[entity]
        if birdData and birdData.isAlive and Coordinator.HasVelocity(entity) then
            local vel = Coordinator.GetVelocity(entity)
            
            -- Apply gravity
            vel.dy = vel.dy + gravData.force * deltaTime
            
            -- Clamp to terminal velocity
            if vel.dy > gravData.terminalVelocity then
                vel.dy = gravData.terminalVelocity
            end
            
            -- Sync ECS::Velocity -> ::Velocity for MovementSystem
            Flappy.SetVelocity(entity, vel.dx, vel.dy)
        end
    end
end

function UpdateMovement(deltaTime)
    -- Birds
    for playerId, entity in pairs(Game.birds) do
        if Coordinator.HasTransform(entity) and Coordinator.HasVelocity(entity) then
            local pos = Coordinator.GetTransform(entity)
            local vel = Coordinator.GetVelocity(entity)
            
            pos.x = pos.x + vel.dx * deltaTime
            pos.y = pos.y + vel.dy * deltaTime
            
            -- Sync Transform -> Position for RenderSystem
            Flappy.SetPosition(entity, pos.x, pos.y)
        end
    end
    
    -- Pipes
    for _, entity in ipairs(Game.pipes) do
        if Coordinator.HasTransform(entity) and Coordinator.HasVelocity(entity) then
            local pos = Coordinator.GetTransform(entity)
            local vel = Coordinator.GetVelocity(entity)
            
            pos.x = pos.x + vel.dx * deltaTime
            pos.y = pos.y + vel.dy * deltaTime
            
            -- Sync Transform -> Position for RenderSystem
            Flappy.SetPosition(entity, pos.x, pos.y)
        end
    end
end

function UpdatePipeSpawning(deltaTime)
    Game.pipeSpawnTimer = Game.pipeSpawnTimer + deltaTime
    
    if Game.pipeSpawnTimer >= Game.pipeSpawnInterval then
        Game.pipeSpawnTimer = 0
        
        -- Random gap position (with margins)
        local minGap = 100
        local maxGap = Game.screenHeight - Game.pipeGapHeight - 100
        local gapY = math.random(minGap, maxGap)
        
        CreatePipePair(gapY)
    end
end

function UpdateCollisions()
    for playerId, birdEntity in pairs(Game.birds) do
        local birdData = Components.FlappyBird[birdEntity]
        if not birdData or not birdData.isAlive then goto continue end
        
        local birdPos = Coordinator.GetTransform(birdEntity)
        local birdSize = Game.birdSize * 1.5  -- Account for scale
        
        -- Check against all pipes
        for _, pipeEntity in ipairs(Game.pipes) do
            local pipeData = Components.Pipe[pipeEntity]
            if not pipeData then goto nextPipe end
            
            local pipePos = Coordinator.GetTransform(pipeEntity)
            
            -- Simple AABB collision
            local birdLeft = birdPos.x
            local birdRight = birdPos.x + birdSize
            local birdTop = birdPos.y
            local birdBottom = birdPos.y + birdSize
            
            local pipeLeft = pipePos.x
            local pipeRight = pipePos.x + pipeData.width
            local pipeTop = pipePos.y
            local pipeBottom = pipePos.y + pipeData.height
            
            if birdRight > pipeLeft and birdLeft < pipeRight and
               birdBottom > pipeTop and birdTop < pipeBottom then
                -- Collision!
                print("💥 Player " .. playerId .. " hit a pipe!")
                KillBird(playerId)
                goto continue
            end
            
            ::nextPipe::
        end
        
        ::continue::
    end
end

function UpdateScoring()
    for playerId, birdEntity in pairs(Game.birds) do
        local birdData = Components.FlappyBird[birdEntity]
        if not birdData or not birdData.isAlive then goto continue end
        
        local birdPos = Coordinator.GetTransform(birdEntity)
        
        -- Check if bird passed any pipes
        for _, pipeEntity in ipairs(Game.pipes) do
            local pipeData = Components.Pipe[pipeEntity]
            if not pipeData or pipeData.passed or not pipeData.isTop then goto nextPipe end
            
            local pipePos = Coordinator.GetTransform(pipeEntity)
            
            -- Bird passed the pipe (bird's left edge > pipe's right edge)
            if birdPos.x > pipePos.x + pipeData.width then
                pipeData.passed = true
                Game.scores[playerId] = (Game.scores[playerId] or 0) + 1
                print("⭐ Player " .. playerId .. " scored! Total: " .. Game.scores[playerId])
                
                -- Check for win condition (30 points)
                if Game.scores[playerId] >= 30 then
                    print("🏆 Player " .. playerId .. " reached 30 points and WON!")
                    SetGameState(GameState.GAMEOVER)
                    return  -- Exit scoring immediately
                end
                
                -- Update UI if this is the local player (with nil check)
                if playerId == Game.localPlayerId and UI and UI.updateScore then
                    UI.updateScore(Game.scores[playerId])
                end
            end
            
            ::nextPipe::
        end
        
        ::continue::
    end
end

function CleanupPipes()
    local toRemove = {}
    
    for i, entity in ipairs(Game.pipes) do
        if Coordinator.HasTransform(entity) then
            local pos = Coordinator.GetTransform(entity)
            if pos.x < -100 then
                table.insert(toRemove, i)
            end
        end
    end
    
    -- Remove from back to front to preserve indices
    for i = #toRemove, 1, -1 do
        local idx = toRemove[i]
        local entity = Game.pipes[idx]
        Flappy.DestroyEntity(entity)
        Components.Pipe[entity] = nil
        table.remove(Game.pipes, idx)
    end
end

function CheckBounds()
    for playerId, entity in pairs(Game.birds) do
        local birdData = Components.FlappyBird[entity]
        if not birdData or not birdData.isAlive then goto continue end
        
        if Coordinator.HasTransform(entity) then
            local pos = Coordinator.GetTransform(entity)
            
            -- Hit ceiling or floor
            if pos.y < 0 or pos.y > Game.screenHeight - Game.birdSize then
                print("💥 Player " .. playerId .. " hit the bounds!")
                KillBird(playerId)
            end
        end
        
        ::continue::
    end
end

-- ============================================
-- RENDER (called every frame from C++)
-- ============================================
function Render()
    -- Custom rendering can be done here if needed
    -- Most rendering is handled by C++ based on Sprite components
end

-- ============================================
-- INPUT CALLBACKS (called from C++)
-- ============================================
function OnKeyPressed(keyCode)
    -- Additional key handling if needed
end

function OnKeyReleased(keyCode)
    -- Additional key handling if needed
end

-- ============================================
-- NETWORK PACKET HANDLING
-- ============================================
function HandleNetworkPacket(packet)
    if not packet or not packet.type then return end
    
    local packetType = packet.type
    
    -- WELCOME (assigns player ID)
    if packetType == PacketType.WELCOME then
        if packet.size >= 1 then
            local playerId = packet.data[1]
            print("👋 [Network] Welcome! Assigned player ID: " .. playerId)
            Game.localPlayerId = playerId
            Game.opponentPlayerId = (playerId == 1) and 2 or 1
        end
    
    -- GAME_START (countdown begins)
    elseif packetType == PacketType.GAME_START then
        if packet.size >= 1 then
            local countdownSeconds = packet.data[1]
            print("🎮 [Network] Game starting! Countdown: " .. countdownSeconds)
            
            if Game.state == GameState.WAITING then
                Game.waitingForOpponent = false
                SetGameState(GameState.COUNTDOWN)
            end
            
            -- Update countdown display
            Game.countdown = countdownSeconds
            if UI and UI.updateCountdown then
                UI.updateCountdown(countdownSeconds)
            end
        end
    
    -- COUNTDOWN_UPDATE
    elseif packetType == PacketType.COUNTDOWN_UPDATE then
        if packet.size >= 1 then
            local countdownSeconds = packet.data[1]
            print("⏱️ [Network] Countdown: " .. countdownSeconds)
            Game.countdown = countdownSeconds
            
            if UI and UI.updateCountdown then
                UI.updateCountdown(countdownSeconds)
            end
            
            -- If countdown reached 0, start playing
            if countdownSeconds == 0 then
                SetGameState(GameState.PLAYING)
            end
        end
    
    -- SPAWN_PIPE (server authoritative pipe spawning)
    elseif packetType == PacketType.SPAWN_PIPE then
        if packet.size >= 2 then
            local x = packet.data[1]
            local gapY = packet.data[2]
            print("🎋 [Network] Spawning pipe at x=" .. x .. ", gapY=" .. gapY)
            
            -- Create top and bottom pipes
            CreatePipeFromServer(x, gapY)
        end
    
    -- GAME_STATE (full game state update)
    elseif packetType == PacketType.GAME_STATE then
        if packet.size >= 1 then
            local numPlayers = packet.data[1]
            local offset = 2
            
            -- Parse each player's state
            for i = 1, numPlayers do
                if offset + 10 <= packet.size then
                    local playerId = packet.data[offset]
                    offset = offset + 1
                    
                    -- Parse y (float, 4 bytes, little-endian)
                    local yBytes = {packet.data[offset], packet.data[offset+1], packet.data[offset+2], packet.data[offset+3]}
                    local y = Network.bytesToFloat(yBytes)
                    offset = offset + 4
                    
                    -- Parse vy (float, 4 bytes, little-endian)
                    local vyBytes = {packet.data[offset], packet.data[offset+1], packet.data[offset+2], packet.data[offset+3]}
                    local vy = Network.bytesToFloat(vyBytes)
                    offset = offset + 4
                    
                    local alive = (packet.data[offset] == 1)
                    offset = offset + 1
                    
                    local score = packet.data[offset]
                    offset = offset + 1
                    
                    -- Update bird
                    if Game.birds[playerId] then
                        UpdateBirdFromServer(playerId, 320, y, vy, alive)
                    end
                    
                    -- Update score
                    Game.scores[playerId] = score
                    
                    -- Update UI if local player's score changed
                    if playerId == Game.localPlayerId and UI and UI.updateScore then
                        UI.updateScore(score)
                    end
                end
            end
        end
    
    -- PLAYER_DIED
    elseif packetType == PacketType.PLAYER_DIED then
        if packet.size >= 1 then
            local playerId = packet.data[1]
            print("💀 [Network] Player " .. playerId .. " died")
            KillBird(playerId)
        end
    
    -- SCORE_UPDATE
    elseif packetType == PacketType.SCORE_UPDATE then
        if packet.size >= 2 then
            local playerId = packet.data[1]
            local score = packet.data[2]
            print("🎯 [Network] Player " .. playerId .. " score: " .. score)
            
            Game.scores[playerId] = score
            
            -- Update UI if it's our score
            if playerId == Game.localPlayerId and UI and UI.updateScore then
                UI.updateScore(score)
            end
        end
    
    -- GAME_OVER
    elseif packetType == PacketType.GAME_OVER then
        if packet.size >= 1 then
            local winnerId = packet.data[1]
            print("🏆 [Network] Game Over! Winner: Player " .. winnerId)
            
            -- Build leaderboard
            local leaderboard = {
                {
                    playerId = winnerId,
                    name = "Player " .. winnerId,
                    score = Game.scores[winnerId] or 0,
                    rank = 1
                },
                {
                    playerId = (winnerId == 1) and 2 or 1,
                    name = "Player " .. ((winnerId == 1) and 2 or 1),
                    score = Game.scores[(winnerId == 1) and 2 or 1] or 0,
                    rank = 2
                }
            }
            
            -- Show game over UI
            if UI and UI.showGameOver then
                UI.showGameOver(leaderboard)
            end
            
            SetGameState(GameState.GAMEOVER)
        end
    
    -- PLAYER_DISCONNECT
    elseif packetType == PacketType.PLAYER_DISCONNECT then
        if packet.size >= 1 then
            local playerId = packet.data[1]
            print("🚪 [Network] Player " .. playerId .. " disconnected")
            
            -- If in game, the remaining player wins
            if Game.state == GameState.PLAYING or Game.state == GameState.COUNTDOWN then
                local winnerId = (playerId == 1) and 2 or 1
                print("   Remaining player " .. winnerId .. " wins by default!")
                
                -- Trigger game over
                local leaderboard = {
                    {
                        playerId = winnerId,
                        name = "Player " .. winnerId .. " (Winner)",
                        score = Game.scores[winnerId] or 0,
                        rank = 1
                    },
                    {
                        playerId = playerId,
                        name = "Player " .. playerId .. " (Disconnected)",
                        score = Game.scores[playerId] or 0,
                        rank = 2
                    }
                }
                
                if UI and UI.showGameOver then
                    UI.showGameOver(leaderboard)
                end
                
                SetGameState(GameState.GAMEOVER)
            end
        end
    
    else
        print("❓ [Network] Unknown packet type: " .. packetType)
    end
end

-- ============================================
-- UI/GAMESTATE CALLBACKS (called from UI buttons)
-- ============================================
function OnPlayLocal()
    print("Starting local game...")
    Game.mode = "local"
    SetGameState(GameState.COUNTDOWN)
end

function OnPlayMultiplayer()
    print("🌐 [Multiplayer] Starting multiplayer mode...")
    Game.mode = "network"
    
    -- Check if Network module exists
    if not Network then
        print("❌ [Multiplayer] Network module not available!")
        SetGameState(GameState.MENU)
        return
    end
    
    -- Initialize network client
    if Network.init then
        print("🔧 [Multiplayer] Initializing NetworkClient...")
        local success = Network.init("127.0.0.1", 8888)
        if not success then
            print("❌ [Multiplayer] Failed to initialize NetworkClient!")
            SetGameState(GameState.MENU)
            return
        end
        print("✅ [Multiplayer] NetworkClient initialized!")
    end
    
    -- Connect to server
    if not Network.connect then
        print("❌ [Multiplayer] Network.connect not available!")
        SetGameState(GameState.MENU)
        return
    end
    
    print("🔌 [Multiplayer] Connecting to server...")
    local success = Network.connect("127.0.0.1", 8888)
    
    if not success then
        print("❌ [Multiplayer] Failed to connect to server!")
        SetGameState(GameState.MENU)
        return
    end
    
    print("✅ [Multiplayer] Connected to server!")
    Game.isConnectedToServer = true
    
    -- Transition to WAITING state
    SetGameState(GameState.WAITING)
end

function OnQuit()
    print("Quitting game...")
    if Flappy and Flappy.Quit then
        Flappy.Quit()
    end
end

function OnReturnToMenu()
    if Game.mode == "network" and Network and Network.disconnect then
        Network.disconnect()
    end
    Game.mode = "local"
    SetGameState(GameState.MENU)
end

-- Expose GameState callbacks to UI
GameState = {
    MENU = "MENU",
    WAITING = "WAITING",
    COUNTDOWN = "COUNTDOWN",
    PLAYING = "PLAYING",
    GAMEOVER = "GAMEOVER",
    
    -- Functions
    Start = function(mode)
        if mode == "local" then
            OnPlayLocal()
        else
            OnPlayMultiplayer()
        end
    end,
    
    Quit = OnQuit
}

-- ============================================
-- UTILITY
-- ============================================
function math.clamp(val, min, max)
    if val < min then return min end
    if val > max then return max end
    return val
end

print("🐦 [Lua] Flappy Bird scripts loaded!")
