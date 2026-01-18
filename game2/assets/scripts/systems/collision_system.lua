-- ============================================================================
-- FLAPPY BIRD - COLLISION SYSTEM
-- ============================================================================
-- Handles collision detection between birds and obstacles (pipes, bounds).
-- Works in conjunction with C++ CollisionSystem but adds game-specific logic.
-- ============================================================================

print("💥 Loading Collision System...")

CollisionSystem = {
    -- Bird collision box (smaller than sprite for forgiveness)
    birdHitboxSize = 24,      -- Square hitbox (bird is ~34x24, we make it smaller)
    birdHitboxOffset = 5,     -- Offset from sprite origin
    
    -- Debug visualization
    debugMode = false,
    
    -- Collision tracking
    recentCollisions = {},    -- Prevent duplicate collision handling
}

-- ============================================================================
-- INITIALIZATION
-- ============================================================================

function CollisionSystem:init()
    print("  🔧 Initializing Collision System...")
    print(string.format("  📐 Bird hitbox: %dx%d (offset: %d)", 
        self.birdHitboxSize, self.birdHitboxSize, self.birdHitboxOffset))
    
    self.recentCollisions = {}
end

-- ============================================================================
-- UPDATE LOOP
-- ============================================================================

function CollisionSystem:update(deltaTime)
    -- Check collisions for all alive birds
    for entityId, birdComp in pairs(Components.FlappyBird) do
        if birdComp.isAlive then
            -- Check bird vs pipes
            self:checkBirdVsPipes(entityId)
            
            -- Check bird vs bounds (handled by GravitySystem)
            -- No need to duplicate here
        end
    end
    
    -- Clean up old collision tracking
    self:cleanupCollisionTracking()
end

-- ============================================================================
-- BIRD vs PIPES COLLISION
-- ============================================================================

-- Check if a bird collides with any pipes
-- @param birdEntityId: the bird entity to check
function CollisionSystem:checkBirdVsPipes(birdEntityId)
    if not Coordinator:HasComponent_Position(birdEntityId) then
        return
    end
    
    local birdPos = Coordinator:GetComponent_Position(birdEntityId)
    
    -- Check against all pipe entities
    for pipeEntityId, pipeComp in pairs(Components.Pipe) do
        if Coordinator:HasComponent_Position(pipeEntityId) and 
           Coordinator:HasComponent_Collider(pipeEntityId) then
            
            local pipePos = Coordinator:GetComponent_Position(pipeEntityId)
            local pipeCollider = Coordinator:GetComponent_Collider(pipeEntityId)
            
            -- Check collision
            if self:checkBirdVsPipe(birdPos, pipePos, pipeCollider) then
                -- Collision detected!
                self:onBirdPipeCollision(birdEntityId, pipeEntityId)
                return  -- One collision is enough
            end
        end
    end
end

-- Check collision between a bird and a pipe (AABB)
-- @param birdPos: bird Position component
-- @param pipePos: pipe Position component
-- @param pipeCollider: pipe Collider component
-- @return true if collision detected
function CollisionSystem:checkBirdVsPipe(birdPos, pipePos, pipeCollider)
    -- Bird hitbox (centered, smaller than sprite)
    local birdLeft = birdPos.x + self.birdHitboxOffset
    local birdRight = birdLeft + self.birdHitboxSize
    local birdTop = birdPos.y + self.birdHitboxOffset
    local birdBottom = birdTop + self.birdHitboxSize
    
    -- Pipe hitbox
    local pipeLeft = pipePos.x + pipeCollider.offsetX
    local pipeRight = pipeLeft + pipeCollider.width
    local pipeTop = pipePos.y + pipeCollider.offsetY
    local pipeBottom = pipeTop + pipeCollider.height
    
    -- AABB collision detection
    return birdLeft < pipeRight and
           birdRight > pipeLeft and
           birdTop < pipeBottom and
           birdBottom > pipeTop
end

-- ============================================================================
-- GENERIC COLLISION CHECK
-- ============================================================================

-- Generic collision check between two entities (with Collider components)
-- @param entityA: first entity ID
-- @param entityB: second entity ID
-- @return true if collision detected
function checkCollision(entityA, entityB)
    if not Coordinator:HasComponent_Position(entityA) or 
       not Coordinator:HasComponent_Position(entityB) or
       not Coordinator:HasComponent_Collider(entityA) or
       not Coordinator:HasComponent_Collider(entityB) then
        return false
    end
    
    local posA = Coordinator:GetComponent_Position(entityA)
    local posB = Coordinator:GetComponent_Position(entityB)
    local colliderA = Coordinator:GetComponent_Collider(entityA)
    local colliderB = Coordinator:GetComponent_Collider(entityB)
    
    -- AABB collision
    local aLeft = posA.x + colliderA.offsetX
    local aRight = aLeft + colliderA.width
    local aTop = posA.y + colliderA.offsetY
    local aBottom = aTop + colliderA.height
    
    local bLeft = posB.x + colliderB.offsetX
    local bRight = bLeft + colliderB.width
    local bTop = posB.y + colliderB.offsetY
    local bBottom = bTop + colliderB.height
    
    return aLeft < bRight and
           aRight > bLeft and
           aTop < bBottom and
           aBottom > bTop
end

-- ============================================================================
-- COLLISION RESPONSE
-- ============================================================================

-- Handle bird-pipe collision
-- @param birdEntityId: the bird that collided
-- @param pipeEntityId: the pipe that was hit
function CollisionSystem:onBirdPipeCollision(birdEntityId, pipeEntityId)
    -- Check if this collision was recently handled (prevent double-processing)
    local collisionKey = string.format("%d_%d", birdEntityId, pipeEntityId)
    if self.recentCollisions[collisionKey] then
        return
    end
    
    -- Mark collision as handled
    self.recentCollisions[collisionKey] = true
    
    -- Get bird component
    local birdComp = Components.FlappyBird[birdEntityId]
    if not birdComp then return end
    
    -- Kill the bird
    print(string.format("💥 Bird %d (Player %d) hit pipe %d!", 
        birdEntityId, birdComp.playerId, pipeEntityId))
    
    -- Use GravitySystem to kill the bird (handles all death logic)
    if GravitySystem then
        GravitySystem:killBird(birdEntityId, "pipe")
    else
        -- Fallback if GravitySystem not loaded yet
        birdComp.isAlive = false
    end
    
    -- Notify scoring system (optional callback)
    if onBirdDeath then
        onBirdDeath(birdEntityId, "pipe")
    end
    
    -- TODO: Play death sound
    -- TODO: Spawn particle effect
end

-- Callback for when a bird dies (can be called from other systems)
-- @param birdEntityId: the bird that died
-- @param reason: death reason ("pipe", "ground", "ceiling")
function onBirdDeath(birdEntityId, reason)
    reason = reason or "unknown"
    
    local birdComp = Components.FlappyBird[birdEntityId]
    if not birdComp then return end
    
    print(string.format("☠️  Bird %d (Player %d) died by %s! Final score: %d",
        birdEntityId, birdComp.playerId, reason, birdComp.score))
    
    -- Update game state
    local gameState = Components.GameState[0]
    if gameState then
        gameState.alivePlayers = math.max(0, gameState.alivePlayers - 1)
        
        -- Check for game over
        if gameState.alivePlayers <= 1 and gameState.state == "playing" then
            print("🏁 Game Over condition met!")
            -- ScoreSystem will handle the actual game over logic
        end
    end
end

-- ============================================================================
-- COLLISION TRACKING CLEANUP
-- ============================================================================

-- Clean up old collision tracking entries
function CollisionSystem:cleanupCollisionTracking()
    -- Simple approach: clear all after a short time
    -- (In practice, collisions are one-time events since birds die)
    -- This prevents memory leaks in long-running games
    
    -- Clear if too many entries
    local count = 0
    for _ in pairs(self.recentCollisions) do
        count = count + 1
    end
    
    if count > 100 then
        self.recentCollisions = {}
    end
end

-- ============================================================================
-- UTILITY FUNCTIONS
-- ============================================================================

-- Enable/disable debug mode
function CollisionSystem:setDebugMode(enabled)
    self.debugMode = enabled
    if enabled then
        print("  🐛 Collision debug mode ENABLED")
    else
        print("  🐛 Collision debug mode DISABLED")
    end
end

-- Adjust bird hitbox size (for difficulty tuning)
function CollisionSystem:setBirdHitboxSize(size)
    self.birdHitboxSize = math.max(10, math.min(40, size))
    print(string.format("  📐 Bird hitbox size changed to %d", self.birdHitboxSize))
end

-- ============================================================================
-- DEBUG FUNCTIONS
-- ============================================================================

function CollisionSystem:debugPrint()
    print("=== Collision System Debug ===")
    print(string.format("  Debug mode: %s", tostring(self.debugMode)))
    print(string.format("  Bird hitbox: %dx%d", self.birdHitboxSize, self.birdHitboxSize))
    print(string.format("  Recent collisions: %d", 
        (function() local c=0; for _ in pairs(self.recentCollisions) do c=c+1 end return c end)()))
end

-- Visualize hitboxes (if debug renderer exists)
function CollisionSystem:debugDrawHitboxes()
    if not self.debugMode then return end
    
    -- TODO: Implement debug rendering
    -- This would require access to the renderer
    -- For now, just print positions
    
    for entityId, birdComp in pairs(Components.FlappyBird) do
        if birdComp.isAlive and Coordinator:HasComponent_Position(entityId) then
            local pos = Coordinator:GetComponent_Position(entityId)
            print(string.format("  Bird %d hitbox: (%.1f, %.1f) - (%.1f, %.1f)",
                entityId,
                pos.x + self.birdHitboxOffset,
                pos.y + self.birdHitboxOffset,
                pos.x + self.birdHitboxOffset + self.birdHitboxSize,
                pos.y + self.birdHitboxOffset + self.birdHitboxSize))
        end
    end
end

print("✅ Collision System loaded!")
print("   - Bird vs Pipes collision detection")
print("   - Death handling and callbacks")

return CollisionSystem
