-- ============================================================================
-- FLAPPY BIRD - PIPE SYSTEM
-- ============================================================================
-- Manages pipe spawning, movement, scoring, and cleanup.
-- Pipes continuously spawn from the right and move left.
-- ============================================================================

print("Loading Pipe System...")

-- Make sure pipe entity factory is loaded
if not createPipePair then
    print("Warning: Pipe entity factory not loaded yet!")
end

-- System state
PipeSystem = {
    -- Spawning configuration
    spawnInterval = 2.5,      -- Seconds between pipe spawns
    timeSinceLastSpawn = 0,   -- Accumulator
    
    -- Pipe tracking
    activePipes = {},         -- List of active pipe pair IDs
    
    -- Scoring
    pipesPassed = {},         -- pipeId -> {playerId1, playerId2, ...}
    
    -- Game state
    isActive = false,         -- Only spawn when game is active
    pipeCount = 0,            -- Total pipes spawned (for stats)
}

-- ============================================================================
-- INITIALIZATION
-- ============================================================================

function PipeSystem:init()
    print("  Initializing Pipe System...")
    self.timeSinceLastSpawn = 0
    self.activePipes = {}
    self.pipesPassed = {}
    self.pipeCount = 0
    self.isActive = false
    
    print(string.format("  Spawn interval: %.1fs", self.spawnInterval))
end

-- Start the pipe system (begin spawning)
function PipeSystem:start()
    self.isActive = true
    self.timeSinceLastSpawn = self.spawnInterval  -- Spawn immediately
    print("  ▶Pipe System started (spawning enabled)")
end

-- Stop the pipe system (pause spawning)
function PipeSystem:stop()
    self.isActive = false
    print("  ⏸Pipe System stopped (spawning disabled)")
end

-- Reset the pipe system (clear all pipes)
function PipeSystem:reset()
    print("  Resetting Pipe System...")
    
    -- Destroy all active pipes
    for _, pairId in ipairs(self.activePipes) do
        -- Each pair ID is the top pipe entity ID
        -- We need to find and destroy both top and bottom
        local pipeComp = Components.Pipe[pairId]
        if pipeComp then
            -- Find all pipes with this component (top and bottom)
            for entityId, comp in pairs(Components.Pipe) do
                if comp == pipeComp then
                    destroyPipe(entityId)
                end
            end
        end
    end
    
    -- Clear tracking
    self.activePipes = {}
    self.pipesPassed = {}
    self.timeSinceLastSpawn = 0
    self.pipeCount = 0
    
    print("  Pipe System reset complete")
end

-- ============================================================================
-- UPDATE LOOP
-- ============================================================================

function PipeSystem:update(deltaTime)
    -- Only spawn if active
    if self.isActive then
        self:updateSpawning(deltaTime)
    end
    
    -- Always update pipe movement and cleanup
    self:updatePipes(deltaTime)
    self:checkScoring()
end

-- Update pipe spawning timer
function PipeSystem:updateSpawning(deltaTime)
    self.timeSinceLastSpawn = self.timeSinceLastSpawn + deltaTime
    
    if self.timeSinceLastSpawn >= self.spawnInterval then
        self:spawnPipePair()
        self.timeSinceLastSpawn = 0
    end
end

-- Update all active pipes (movement, cleanup)
function PipeSystem:updatePipes(deltaTime)
    -- Note: Movement is handled by MovementSystem (Velocity component)
    -- We just need to handle cleanup and state updates
    
    local pipesToRemove = {}
    
    for i, pairId in ipairs(self.activePipes) do
        -- Check if pipe is off-screen
        if isPipeOffScreen(pairId) then
            table.insert(pipesToRemove, i)
            
            -- Destroy the pipe pair
            local pipeComp = Components.Pipe[pairId]
            if pipeComp then
                for entityId, comp in pairs(Components.Pipe) do
                    if comp == pipeComp then
                        destroyPipe(entityId)
                    end
                end
            end
        end
    end
    
    -- Remove destroyed pipes from tracking (reverse order to preserve indices)
    for i = #pipesToRemove, 1, -1 do
        local index = pipesToRemove[i]
        table.remove(self.activePipes, index)
    end
end

-- ============================================================================
-- SPAWNING
-- ============================================================================

-- Spawn a new pipe pair
function PipeSystem:spawnPipePair()
    local topEntity, bottomEntity, pairId = createRandomPipePair()
    
    -- Track the pair
    table.insert(self.activePipes, pairId)
    self.pipesPassed[pairId] = {}  -- Init scoring tracker
    self.pipeCount = self.pipeCount + 1
    
    print(string.format("  Spawned pipe pair #%d (entities: %d, %d)", 
        self.pipeCount, topEntity, bottomEntity))
    
    return pairId
end

-- ============================================================================
-- SCORING
-- ============================================================================

-- Check if any birds have passed pipes (for scoring)
function PipeSystem:checkScoring()
    -- Iterate through all active pipes
    for _, pairId in ipairs(self.activePipes) do
        local pipeComp = Components.Pipe[pairId]
        if pipeComp and not pipeComp.passed then
            
            -- Check all alive birds
            for entityId, birdComp in pairs(Components.FlappyBird) do
                if birdComp.isAlive then
                    -- Check if this bird passed this pipe
                    if hasBirdPassedPipe(pairId, entityId) then
                        self:awardScore(pairId, entityId, birdComp.playerId)
                    end
                end
            end
        end
    end
end

-- Award score to a bird for passing a pipe
-- @param pairId: the pipe pair ID
-- @param birdEntityId: the bird entity
-- @param playerId: the player ID
function PipeSystem:awardScore(pairId, birdEntityId, playerId)
    local scoredPlayers = self.pipesPassed[pairId]
    if not scoredPlayers then
        scoredPlayers = {}
        self.pipesPassed[pairId] = scoredPlayers
    end
    
    -- Check if this player already scored from this pipe
    for _, pid in ipairs(scoredPlayers) do
        if pid == playerId then
            return  -- Already scored
        end
    end
    
    -- Award score!
    table.insert(scoredPlayers, playerId)
    
    local birdComp = Components.FlappyBird[birdEntityId]
    if birdComp then
        birdComp.score = birdComp.score + 1
        print(string.format("  Player %d scored! (Pipe #%d, Total: %d)", 
            playerId, pairId, birdComp.score))
    end
    
    -- Mark pipe as passed if all alive birds passed it
    local aliveCount = 0
    local passedCount = #scoredPlayers
    
    for _, bird in pairs(Components.FlappyBird) do
        if bird.isAlive then
            aliveCount = aliveCount + 1
        end
    end
    
    if passedCount >= aliveCount then
        local pipeComp = Components.Pipe[pairId]
        if pipeComp then
            pipeComp.passed = true
        end
    end
end

-- ============================================================================
-- UTILITY FUNCTIONS
-- ============================================================================

-- Get number of active pipe pairs
function PipeSystem:getActivePipeCount()
    return #self.activePipes
end

-- Get total pipes spawned
function PipeSystem:getTotalPipesSpawned()
    return self.pipeCount
end

-- Set spawn interval (difficulty adjustment)
function PipeSystem:setSpawnInterval(interval)
    self.spawnInterval = math.max(1.0, interval)  -- Min 1 second
    print(string.format("  Pipe spawn interval changed to %.1fs", self.spawnInterval))
end

-- Increase difficulty (faster spawning)
function PipeSystem:increaseDifficulty()
    self.spawnInterval = math.max(1.5, self.spawnInterval - 0.2)
    print(string.format("  Difficulty increased! Spawn interval: %.1fs", self.spawnInterval))
end

-- ============================================================================
-- DEBUG FUNCTIONS
-- ============================================================================

function PipeSystem:debugPrint()
    print("=== Pipe System Debug ===")
    print(string.format("  Active: %s", tostring(self.isActive)))
    print(string.format("  Active Pipes: %d", #self.activePipes))
    print(string.format("  Total Spawned: %d", self.pipeCount))
    print(string.format("  Time until next: %.2fs", self.spawnInterval - self.timeSinceLastSpawn))
    
    for i, pairId in ipairs(self.activePipes) do
        if Coordinator:HasComponent_Position(pairId) then
            local pos = Coordinator:GetComponent_Position(pairId)
            local pipeComp = Components.Pipe[pairId]
            local passed = pipeComp and pipeComp.passed or false
            print(string.format("  Pipe %d: x=%.1f, passed=%s", i, pos.x, tostring(passed)))
        end
    end
end

print("Pipe System loaded!")
print("   - Spawning, movement, scoring, cleanup")

return PipeSystem
