-- network.lua
-- Flappy Bird Battle Royale Network Client

local Network = {}

-- State
Network.connected = false
Network.playerId = nil
Network.client = nil
Network.stateCallback = nil
Network.gameOverCallback = nil

-- Connect to the game server
function Network.connect(serverIp, port)
    print("[Network] Connecting to " .. serverIp .. ":" .. port)
    
    -- Create network client (assuming NetworkClient is available via C++ bindings)
    if not NetworkClient then
        print("[Network] ERROR: NetworkClient not available")
        return false
    end
    
    Network.client = NetworkClient.new()
    local success = Network.client:connect(serverIp, port)
    
    if success then
        Network.connected = true
        print("[Network] Connected to server!")
        
        -- Start receiving packets in a coroutine/update loop
        Network.startReceiving()
        return true
    else
        print("[Network] Failed to connect to server")
        return false
    end
end

-- Send a flap input to the server
function Network.sendFlap()
    if not Network.connected or not Network.client then
        print("[Network] Cannot send flap: not connected")
        return
    end
    
    -- Send FLAPPY_INPUT packet
    -- Packet format: { type = "FLAPPY_INPUT", playerId = ..., flapped = true }
    local packet = {
        type = "FLAPPY_INPUT",
        playerId = Network.playerId or 0,
        flapped = true
    }
    
    Network.client:sendPacket(packet)
    print("[Network] Sent flap input")
end

-- Disconnect from server
function Network.disconnect()
    if Network.client then
        Network.client:disconnect()
        Network.connected = false
        Network.playerId = nil
        print("[Network] Disconnected from server")
    end
end

-- Register callback for state updates
function Network.onStateReceived(callback)
    Network.stateCallback = callback
end

-- Register callback for game over
function Network.onGameOver(callback)
    Network.gameOverCallback = callback
end

-- Start receiving packets (called from update loop)
function Network.startReceiving()
    print("[Network] Started receiving packets")
end

-- Update network (call this every frame)
function Network.update(dt)
    if not Network.connected or not Network.client then
        return
    end
    
    -- Receive packets
    local packets = Network.client:receivePackets()
    if not packets then
        return
    end
    
    for _, packet in ipairs(packets) do
        Network.handlePacket(packet)
    end
end

-- Handle incoming packets
function Network.handlePacket(packet)
    if not packet or not packet.type then
        return
    end
    
    if packet.type == "FLAPPY_ASSIGN_ID" then
        Network.playerId = packet.playerId
        print("[Network] Assigned player ID: " .. Network.playerId)
        
    elseif packet.type == "FLAPPY_LOBBY_UPDATE" then
        print("[Network] Lobby update: " .. packet.playerCount .. "/" .. packet.maxPlayers .. " players")
        if Network.stateCallback then
            Network.stateCallback({
                state = "lobby",
                playerCount = packet.playerCount,
                maxPlayers = packet.maxPlayers
            })
        end
        
    elseif packet.type == "FLAPPY_COUNTDOWN" then
        print("[Network] Countdown: " .. packet.secondsLeft)
        if Network.stateCallback then
            Network.stateCallback({
                state = "countdown",
                secondsLeft = packet.secondsLeft
            })
        end
        
    elseif packet.type == "FLAPPY_GAME_START" then
        print("[Network] Game started!")
        if Network.stateCallback then
            Network.stateCallback({
                state = "playing"
            })
        end
        
    elseif packet.type == "FLAPPY_STATE" then
        -- Update all player positions
        if Network.stateCallback then
            Network.stateCallback({
                state = "game_update",
                players = packet.players,
                pipes = packet.pipes
            })
        end
        
    elseif packet.type == "FLAPPY_GAME_OVER" then
        print("[Network] Game over! Winner: Player " .. packet.winnerId)
        if Network.gameOverCallback then
            Network.gameOverCallback({
                winnerId = packet.winnerId,
                leaderboard = packet.leaderboard
            })
        end
        
    elseif packet.type == "FLAPPY_PLAYER_DIED" then
        print("[Network] Player " .. packet.playerId .. " died. Score: " .. packet.score)
        if Network.stateCallback then
            Network.stateCallback({
                state = "player_died",
                playerId = packet.playerId,
                score = packet.score
            })
        end
    end
end

return Network
