-- ============================================================================
-- FLAPPY BIRD - SCORE SYSTEM
-- ============================================================================
-- Manages scoring, leaderboard, and game over detection.
-- Tracks individual player scores and determines winners.
-- ============================================================================

print("Loading Score System...")

ScoreSystem = {
    -- Leaderboard
    leaderboard = {},         -- { {playerId, entityId, score, isAlive}, ... }
    
    -- Game state tracking
    gameStartTime = 0,
    gameDuration = 0,
    
    -- Statistics
    totalScores = 0,
    highestScore = 0,
    winnerId = -1,
}

-- ============================================================================
-- INITIALIZATION
-- ============================================================================

function ScoreSystem:init()
    print("  Initializing Score System...")
    self.leaderboard = {}
    self.gameStartTime = 0
    self.gameDuration = 0
    self.totalScores = 0
    self.highestScore = 0
    self.winnerId = -1
    
    print("  Leaderboard ready")
end

-- Start tracking scores (game started)
function ScoreSystem:start()
    self.gameStartTime = os.clock()  -- Record start time
    self:updateLeaderboard()
    print("  ▶Score tracking started")
end

-- Reset the score system
function ScoreSystem:reset()
    print("  Resetting Score System...")
    self:init()
end

-- ============================================================================
-- UPDATE LOOP
-- ============================================================================

function ScoreSystem:update(deltaTime)
    -- Update game duration
    if self.gameStartTime > 0 then
        self.gameDuration = os.clock() - self.gameStartTime
    end
    
    -- Update leaderboard (scores may have changed)
    self:updateLeaderboard()
    
    -- Check for game over conditions
    self:checkGameOver()
end

-- ============================================================================
-- LEADERBOARD MANAGEMENT
-- ============================================================================

-- Update the leaderboard with current scores
function ScoreSystem:updateLeaderboard()
    self.leaderboard = {}
    self.totalScores = 0
    self.highestScore = 0
    
    -- Collect all birds and their scores
    for entityId, birdComp in pairs(Components.FlappyBird) do
        local entry = {
            playerId = birdComp.playerId,
            entityId = entityId,
            score = birdComp.score,
            isAlive = birdComp.isAlive,
            color = birdComp.color
        }
        
        table.insert(self.leaderboard, entry)
        
        self.totalScores = self.totalScores + birdComp.score
        if birdComp.score > self.highestScore then
            self.highestScore = birdComp.score
        end
    end
    
    -- Sort by score (descending), then by alive status
    table.sort(self.leaderboard, function(a, b)
        if a.score == b.score then
            -- If scores are equal, alive players come first
            if a.isAlive ~= b.isAlive then
                return a.isAlive
            end
            -- If both alive or both dead, sort by player ID
            return a.playerId < b.playerId
        end
        return a.score > b.score
    end)
end

-- Get the current leaderboard
function getLeaderboard()
    if ScoreSystem then
        return ScoreSystem.leaderboard
    end
    return {}
end

-- Get leaderboard as formatted string
function ScoreSystem:getLeaderboardString()
    local str = "=== LEADERBOARD ===\n"
    
    for i, entry in ipairs(self.leaderboard) do
        local status = entry.isAlive and "🐦" or "💀"
        str = str .. string.format("%d. Player %d: %d points %s\n",
            i, entry.playerId, entry.score, status)
    end
    
    return str
end

-- Print the leaderboard to console
function ScoreSystem:printLeaderboard()
    print(self:getLeaderboardString())
end

-- ============================================================================
-- SCORING LOGIC
-- ============================================================================

-- Award points to a player
function ScoreSystem:awardPoints(playerId, points)
    points = points or 1
    
    -- Find the bird with this player ID
    for entityId, birdComp in pairs(Components.FlappyBird) do
        if birdComp.playerId == playerId then
            birdComp.score = birdComp.score + points
            print(string.format("  Player %d scored %d point(s)! Total: %d",
                playerId, points, birdComp.score))
            return
        end
    end
end

-- Award points to a bird entity
function ScoreSystem:awardPointsToEntity(entityId, points)
    points = points or 1
    
    local birdComp = Components.FlappyBird[entityId]
    if birdComp then
        birdComp.score = birdComp.score + points
        print(string.format("  Player %d scored %d point(s)! Total: %d",
            birdComp.playerId, points, birdComp.score))
    end
end

-- Get score for a player
function ScoreSystem:getPlayerScore(playerId)
    for entityId, birdComp in pairs(Components.FlappyBird) do
        if birdComp.playerId == playerId then
            return birdComp.score
        end
    end
    return 0
end

-- ============================================================================
-- GAME OVER DETECTION
-- ============================================================================

-- Check if game over conditions are met
function ScoreSystem:checkGameOver()
    local gameState = Components.GameState[0]
    if not gameState then return end
    
    -- Only check during "playing" state
    if gameState.state ~= "playing" then return end
    
    -- Count alive players
    local aliveCount = 0
    local lastAliveId = -1
    
    for entityId, birdComp in pairs(Components.FlappyBird) do
        if birdComp.isAlive then
            aliveCount = aliveCount + 1
            lastAliveId = birdComp.playerId
        end
    end
    
    -- Game over if 1 or 0 players left
    if aliveCount <= 1 then
        self:triggerGameOver(lastAliveId)
    end
end

-- Trigger game over
function ScoreSystem:triggerGameOver(lastAlivePlayerId)
    local gameState = Components.GameState[0]
    if not gameState then return end
    
    -- Prevent multiple triggers
    if gameState.state == "gameover" then return end
    
    print("=" .. string.rep("=", 50))
    print("GAME OVER!")
    print("=" .. string.rep("=", 50))
    
    -- Determine winner
    self:updateLeaderboard()
    
    if #self.leaderboard > 0 then
        local winner = self.leaderboard[1]
        self.winnerId = winner.playerId
        gameState.winnerId = winner.playerId
        
        print(string.format("WINNER: Player %d with %d points!",
            winner.playerId, winner.score))
    else
        self.winnerId = -1
        gameState.winnerId = -1
        print("All players died!")
    end
    
    -- Print final leaderboard
    self:printLeaderboard()
    
    -- Print statistics
    print(string.format("Game duration: %.1f seconds", self.gameDuration))
    print(string.format("Total scores: %d", self.totalScores))
    print(string.format("Highest score: %d", self.highestScore))
    print("=" .. string.rep("=", 50))
    
    -- Update game state
    gameState.state = "gameover"
    
    -- Stop pipe spawning
    if PipeSystem then
        PipeSystem:stop()
    end
end

-- ============================================================================
-- STATISTICS
-- ============================================================================

-- Get game statistics
function ScoreSystem:getStatistics()
    return {
        duration = self.gameDuration,
        totalScores = self.totalScores,
        highestScore = self.highestScore,
        winnerId = self.winnerId,
        playerCount = #self.leaderboard,
        aliveCount = self:getAlivePlayerCount()
    }
end

-- Get number of alive players
function ScoreSystem:getAlivePlayerCount()
    local count = 0
    for _, birdComp in pairs(Components.FlappyBird) do
        if birdComp.isAlive then
            count = count + 1
        end
    end
    return count
end

-- Get winner info
function ScoreSystem:getWinner()
    if #self.leaderboard > 0 then
        local winner = self.leaderboard[1]
        return {
            playerId = winner.playerId,
            score = winner.score,
            isAlive = winner.isAlive
        }
    end
    return nil
end

-- ============================================================================
-- UTILITY FUNCTIONS
-- ============================================================================

-- Broadcast score update (for network play)
function ScoreSystem:broadcastScoreUpdate(playerId, newScore)
    -- For now, just log it
    print(string.format("  Broadcasting: Player %d score: %d", playerId, newScore))
end

-- ============================================================================
-- DEBUG FUNCTIONS
-- ============================================================================

function ScoreSystem:debugPrint()
    print("=== Score System Debug ===")
    print(string.format("  Game duration: %.1f s", self.gameDuration))
    print(string.format("  Total scores: %d", self.totalScores))
    print(string.format("  Highest score: %d", self.highestScore))
    print(string.format("  Alive players: %d", self:getAlivePlayerCount()))
    print(string.format("  Leaderboard entries: %d", #self.leaderboard))
    
    if #self.leaderboard > 0 then
        print("  Top 3:")
        for i = 1, math.min(3, #self.leaderboard) do
            local entry = self.leaderboard[i]
            print(string.format("    %d. Player %d: %d points %s",
                i, entry.playerId, entry.score, entry.isAlive and "🐦" or "💀"))
        end
    end
end

print("Score System loaded!")
print("   - Leaderboard tracking")
print("   - Game over detection")
print("   - Statistics and winners")

return ScoreSystem
