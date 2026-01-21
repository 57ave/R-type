-- ============================================================================
-- SCORE MANAGER
-- Handles local score persistence and high score tracking
-- ============================================================================

ScoreManager = {
    scoresFile = "scores.json",
    highScores = {},
    maxHighScores = 10,
    
    currentGameScore = {
        score = 0,
        wave = 0,
        kills = 0,
        time = 0,
        date = "",
        mode = "solo"
    }
}

-- ============================================================================
-- INITIALIZATION
-- ============================================================================

function ScoreManager:Init()
    print("[ScoreManager] Initializing score system...")
    
    -- Load existing high scores from file
    self:LoadScores()
    
    -- Reset current game score
    self:ResetCurrentGame()
    
    print("[ScoreManager] ✓ Score system initialized")
    return true
end

-- ============================================================================
-- CURRENT GAME TRACKING
-- ============================================================================

function ScoreManager:ResetCurrentGame()
    self.currentGameScore = {
        score = 0,
        wave = 0,
        kills = 0,
        time = 0,
        date = os.date("%Y-%m-%d"),
        mode = "solo"
    }
end

function ScoreManager:UpdateScore(points)
    self.currentGameScore.score = self.currentGameScore.score + points
end

function ScoreManager:UpdateStats(stats)
    if stats.score then
        self.currentGameScore.score = stats.score
    end
    if stats.wave then
        self.currentGameScore.wave = stats.wave
    end
    if stats.kills then
        self.currentGameScore.kills = stats.kills
    end
    if stats.time then
        self.currentGameScore.time = stats.time
    end
end

function ScoreManager:GetCurrentScore()
    return self.currentGameScore
end

-- ============================================================================
-- HIGH SCORES MANAGEMENT
-- ============================================================================

function ScoreManager:SaveGameScore(isVictory)
    local gameData = {
        score = self.currentGameScore.score,
        wave = self.currentGameScore.wave,
        kills = self.currentGameScore.kills,
        time = self.currentGameScore.time,
        date = os.date("%Y-%m-%d %H:%M"),
        mode = self.currentGameScore.mode,
        victory = isVictory or false
    }
    
    -- Add to high scores list
    table.insert(self.highScores, gameData)
    
    -- Sort by score (highest first)
    table.sort(self.highScores, function(a, b)
        return a.score > b.score
    end)
    
    -- Keep only top scores
    while #self.highScores > self.maxHighScores do
        table.remove(self.highScores)
    end
    
    -- Save to file
    self:SaveScores()
    
    print("[ScoreManager] Game score saved: " .. gameData.score)
    
    return self:GetScoreRank(gameData.score)
end

function ScoreManager:GetScoreRank(score)
    for i, scoreData in ipairs(self.highScores) do
        if scoreData.score == score then
            return i
        end
    end
    return nil
end

function ScoreManager:GetHighScores()
    return self.highScores
end

function ScoreManager:GetBestScore()
    if #self.highScores > 0 then
        return self.highScores[1].score
    end
    return 0
end

-- ============================================================================
-- FILE I/O
-- ============================================================================

function ScoreManager:LoadScores()
    print("[ScoreManager] Loading scores from file...")
    
    -- Try to load from C++ binding if available
    if LoadScoresFromFile then
        local scoresData = LoadScoresFromFile(self.scoresFile)
        if scoresData then
            self.highScores = scoresData.high_scores or {}
            print("[ScoreManager] ✓ Loaded " .. #self.highScores .. " high scores")
            return true
        end
    end
    
    -- Fallback: try to load manually
    local file = io.open(self.scoresFile, "r")
    if file then
        local content = file:read("*all")
        file:close()
        
        -- Parse JSON manually (basic implementation)
        -- This is a simplified version - in production use a proper JSON library
        -- For now, just initialize empty
        self.highScores = {}
        print("[ScoreManager] ⚠️ Manual JSON parsing not implemented, starting fresh")
    else
        print("[ScoreManager] No existing scores file, starting fresh")
        self.highScores = {}
    end
    
    return true
end

function ScoreManager:SaveScores()
    print("[ScoreManager] Saving scores to file...")
    
    -- Try to save using C++ binding if available
    if SaveScoresToFile then
        local scoresData = {
            high_scores = self.highScores
        }
        SaveScoresToFile(self.scoresFile, scoresData)
        print("[ScoreManager] ✓ Scores saved via C++ binding")
        return true
    end
    
    -- Fallback: save manually
    self:SaveScoresManual()
    
    return true
end

function ScoreManager:SaveScoresManual()
    local file = io.open(self.scoresFile, "w")
    if not file then
        print("[ScoreManager] ❌ Could not open scores file for writing")
        return false
    end
    
    -- Build JSON manually
    file:write("{\n")
    file:write('  "high_scores": [\n')
    
    for i, score in ipairs(self.highScores) do
        file:write("    {\n")
        file:write('      "score": ' .. score.score .. ',\n')
        file:write('      "wave": ' .. (score.wave or 0) .. ',\n')
        file:write('      "kills": ' .. (score.kills or 0) .. ',\n')
        file:write('      "time": ' .. (score.time or 0) .. ',\n')
        file:write('      "date": "' .. (score.date or "") .. '",\n')
        file:write('      "mode": "' .. (score.mode or "solo") .. '",\n')
        file:write('      "victory": ' .. tostring(score.victory or false) .. '\n')
        file:write("    }")
        
        if i < #self.highScores then
            file:write(",")
        end
        file:write("\n")
    end
    
    file:write("  ]\n")
    file:write("}\n")
    file:close()
    
    print("[ScoreManager] ✓ Scores saved manually to " .. self.scoresFile)
    return true
end

-- ============================================================================
-- MULTIPLAYER SCORES (Task 16 - Per-player score persistence)
-- ============================================================================

function ScoreManager:SaveMultiplayerScore(playerName, score)
    -- Handle both number and table input
    local personalScore = type(score) == "number" and score or (score.personal_score or 0)
    local teamScore = type(score) == "table" and (score.team_score or 0) or 0
    local waves = type(score) == "table" and (score.waves or 0) or 0
    local victory = type(score) == "table" and (score.victory or false) or false
    
    local playerScoresFile = "scores_" .. playerName .. ".json"
    
    local gameData = {
        mode = "multiplayer",
        playerName = playerName,
        personal_score = personalScore,
        team_score = teamScore,
        date = os.date("%Y-%m-%d %H:%M"),
        waves_completed = waves,
        victory = victory
    }
    
    -- If C++ binding exists, use it
    if SaveMultiplayerScoreToFile then
        SaveMultiplayerScoreToFile(playerScoresFile, gameData)
        print("[ScoreManager] ✓ Multiplayer score saved for " .. playerName .. " (" .. personalScore .. " pts)")
        return true
    end
    
    -- Otherwise, save manually as JSON
    print("[ScoreManager] Saving multiplayer score for " .. playerName)
    
    -- Load existing scores for this player
    local existingScores = self:LoadPlayerScores(playerName)
    table.insert(existingScores, gameData)
    
    -- Keep only last 20 games
    while #existingScores > 20 do
        table.remove(existingScores, 1)
    end
    
    -- Write to file
    local file = io.open(playerScoresFile, "w")
    if file then
        file:write("{\n")
        file:write('  "player": "' .. playerName .. '",\n')
        file:write('  "games": [\n')
        
        for i, game in ipairs(existingScores) do
            file:write("    {\n")
            file:write('      "mode": "' .. game.mode .. '",\n')
            file:write('      "personal_score": ' .. game.personal_score .. ',\n')
            file:write('      "team_score": ' .. game.team_score .. ',\n')
            file:write('      "date": "' .. game.date .. '",\n')
            file:write('      "waves_completed": ' .. game.waves_completed .. ',\n')
            file:write('      "victory": ' .. tostring(game.victory) .. '\n')
            file:write("    }")
            if i < #existingScores then
                file:write(",")
            end
            file:write("\n")
        end
        
        file:write("  ]\n")
        file:write("}\n")
        file:close()
        
        print("[ScoreManager] ✓ Written " .. #existingScores .. " games for " .. playerName)
    else
        print("[ScoreManager] ⚠️ Failed to open " .. playerScoresFile .. " for writing")
    end
    
    return true
end

function ScoreManager:LoadPlayerScores(playerName)
    local playerScoresFile = "scores_" .. playerName .. ".json"
    local scores = {}
    
    local file = io.open(playerScoresFile, "r")
    if file then
        file:close()
        -- Simple parser - in production, use proper JSON library
        print("[ScoreManager] Found existing scores for " .. playerName)
        -- For now, return empty to avoid complex parsing
    end
    
    return scores
end

-- ============================================================================
-- FORMATTING HELPERS
-- ============================================================================

function ScoreManager:FormatTime(seconds)
    local mins = math.floor(seconds / 60)
    local secs = seconds % 60
    return string.format("%02d:%02d", mins, secs)
end

function ScoreManager:FormatScore(score)
    -- Add comma separators for readability
    local formatted = tostring(score)
    local k
    while true do  
        formatted, k = string.gsub(formatted, "^(-?%d+)(%d%d%d)", '%1,%2')
        if k == 0 then
            break
        end
    end
    return formatted
end

-- ============================================================================
-- DEBUG
-- ============================================================================

function ScoreManager:PrintHighScores()
    print("[ScoreManager] ========== HIGH SCORES ==========")
    for i, score in ipairs(self.highScores) do
        print(string.format("  %d. %s - Wave %d - %s", 
            i, 
            self:FormatScore(score.score), 
            score.wave or 0,
            score.date or ""))
    end
    print("[ScoreManager] ================================")
end

print("[ScoreManager] Score Manager system loaded")

return ScoreManager
