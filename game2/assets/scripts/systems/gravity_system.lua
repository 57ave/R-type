-- ============================================================================
-- FLAPPY BIRD - GRAVITY SYSTEM
-- ============================================================================
-- Handles gravity physics, flapping, and death by screen boundaries.
-- This system works with both C++ ECS components (Position, Velocity) 
-- and Lua components (Gravity, FlappyBird).
-- ============================================================================

print("⬇️  Loading Gravity System...")

-- System configuration
GravitySystem = {
    -- Screen boundaries
    screenWidth = 1280,
    screenHeight = 720,
    groundY = 680,      -- Y position of ground (with some margin)
    ceilingY = 40,      -- Y position of ceiling
    
    -- Physics constants (can be overridden)
    defaultGravity = 980.0,         -- pixels/s² (roughly Earth gravity)
    defaultFlapStrength = 350.0,    -- upward velocity on flap (pixels/s)
    defaultTerminalVelocity = 600.0, -- max fall speed (pixels/s)
    
    -- Flap cooldown (prevent spam)
    flapCooldown = 0.15,  -- seconds between flaps
    lastFlapTime = {}     -- entityId -> last flap timestamp
}

-- ============================================================================
-- INITIALIZATION
-- ============================================================================

function GravitySystem:init()
    print("  🔧 Initializing Gravity System...")
    self.lastFlapTime = {}
    print(string.format("  📐 Screen bounds: %dx%d (ground: %.1f, ceiling: %.1f)", 
        self.screenWidth, self.screenHeight, self.groundY, self.ceilingY))
    print(string.format("  ⚙️  Physics: gravity=%.1f, flap=%.1f, terminal=%.1f", 
        self.defaultGravity, self.defaultFlapStrength, self.defaultTerminalVelocity))
end

-- ============================================================================
-- UPDATE LOOP
-- ============================================================================

-- Main update function called every frame
-- @param deltaTime: time since last frame in seconds
function GravitySystem:update(deltaTime)
    -- Iterate over all entities with Gravity component
    for entityId, gravityComp in pairs(Components.Gravity) do
        -- Check if entity also has Position and Velocity (C++ components)
        if Coordinator:HasComponent_Position(entityId) and 
           Coordinator:HasComponent_Velocity(entityId) then
            
            -- Get C++ components
            local position = Coordinator:GetComponent_Position(entityId)
            local velocity = Coordinator:GetComponent_Velocity(entityId)
            
            -- Apply gravity acceleration
            velocity.vy = velocity.vy + gravityComp.force * deltaTime
            
            -- Clamp to terminal velocity (prevent falling too fast)
            if velocity.vy > gravityComp.terminalVelocity then
                velocity.vy = gravityComp.terminalVelocity
            end
            
            -- Check boundaries (death conditions)
            self:checkBoundaries(entityId, position)
        end
    end
end

-- ============================================================================
-- FLAP MECHANICS
-- ============================================================================

-- Apply a flap (jump) to an entity
-- @param entityId: the entity to flap
-- @param currentTime: current game time (for cooldown check)
-- @return true if flap was successful, false if on cooldown or dead
function GravitySystem:flap(entityId, currentTime)
    currentTime = currentTime or 0
    
    -- Check if entity has FlappyBird component and is alive
    local birdComp = Components.FlappyBird[entityId]
    if not birdComp or not birdComp.isAlive then
        return false
    end
    
    -- Check flap cooldown
    local lastFlap = self.lastFlapTime[entityId] or 0
    if (currentTime - lastFlap) < self.flapCooldown then
        -- Still on cooldown
        return false
    end
    
    -- Check if entity has Velocity component
    if not Coordinator:HasComponent_Velocity(entityId) then
        return false
    end
    
    -- Apply flap!
    local velocity = Coordinator:GetComponent_Velocity(entityId)
    velocity.vy = birdComp.flapStrength  -- Negative = upward
    
    -- Update last flap time
    self.lastFlapTime[entityId] = currentTime
    
    -- Optional: Add flap animation or sound trigger here
    -- print(string.format("🐦 Entity %d flapped! (vy = %.1f)", entityId, velocity.vy))
    
    return true
end

-- Flap with default parameters (no time check - immediate flap)
-- @param entityId: the entity to flap
function GravitySystem:flapImmediate(entityId)
    local birdComp = Components.FlappyBird[entityId]
    if not birdComp or not birdComp.isAlive then
        return false
    end
    
    if not Coordinator:HasComponent_Velocity(entityId) then
        return false
    end
    
    local velocity = Coordinator:GetComponent_Velocity(entityId)
    velocity.vy = birdComp.flapStrength
    
    return true
end

-- ============================================================================
-- BOUNDARY CHECKING
-- ============================================================================

-- Check if entity hit screen boundaries (death condition)
-- @param entityId: entity to check
-- @param position: Position component reference
function GravitySystem:checkBoundaries(entityId, position)
    local birdComp = Components.FlappyBird[entityId]
    
    -- Only check boundaries for alive birds
    if not birdComp or not birdComp.isAlive then
        return
    end
    
    -- Check ceiling
    if position.y <= self.ceilingY then
        self:killBird(entityId, "ceiling")
        position.y = self.ceilingY  -- Clamp position
        return
    end
    
    -- Check ground
    if position.y >= self.groundY then
        self:killBird(entityId, "ground")
        position.y = self.groundY  -- Clamp position
        return
    end
end

-- ============================================================================
-- DEATH HANDLING
-- ============================================================================

-- Kill a bird (mark as dead)
-- @param entityId: the bird to kill
-- @param reason: reason for death ("ceiling", "ground", "pipe")
function GravitySystem:killBird(entityId, reason)
    local birdComp = Components.FlappyBird[entityId]
    if not birdComp then return end
    
    -- Only kill if alive
    if birdComp.isAlive then
        birdComp.isAlive = false
        
        print(string.format("💀 Bird %d (Player %d) died by %s! Final score: %d", 
            entityId, birdComp.playerId, reason or "unknown", birdComp.score))
        
        -- Update game state (decrease alive count)
        local gameState = Components.GameState[0]
        if gameState then
            gameState.alivePlayers = math.max(0, gameState.alivePlayers - 1)
            
            -- Check for game over (only 1 or 0 players left)
            if gameState.alivePlayers <= 1 and gameState.state == "playing" then
                self:handleGameOver()
            end
        end
        
        -- Optional: Stop velocity when dead (fall to ground)
        if Coordinator:HasComponent_Velocity(entityId) then
            local velocity = Coordinator:GetComponent_Velocity(entityId)
            -- Don't zero velocity - let them fall naturally for visual effect
            -- velocity.vy = 0
        end
    end
end

-- Handle game over condition (last bird standing or all dead)
function GravitySystem:handleGameOver()
    local gameState = Components.GameState[0]
    if not gameState then return end
    
    -- Find winner (last alive bird or highest score)
    local winnerId = -1
    local maxScore = -1
    
    for entityId, birdComp in pairs(Components.FlappyBird) do
        if birdComp.isAlive then
            -- This is the winner (last one standing)
            winnerId = birdComp.playerId
            break
        elseif birdComp.score > maxScore then
            -- Track highest score in case all are dead
            maxScore = birdComp.score
            winnerId = birdComp.playerId
        end
    end
    
    gameState.state = "gameover"
    gameState.winnerId = winnerId
    
    print(string.format("🏆 GAME OVER! Winner: Player %d (Score: %d)", 
        winnerId, maxScore))
end

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Set screen dimensions (called from main game)
function GravitySystem:setScreenSize(width, height)
    self.screenWidth = width
    self.screenHeight = height
    self.groundY = height - 40  -- 40 pixels from bottom
    self.ceilingY = 40           -- 40 pixels from top
    
    print(string.format("  📐 Gravity System: Screen updated to %dx%d", width, height))
end

-- Reset cooldowns (useful for game restart)
function GravitySystem:resetCooldowns()
    self.lastFlapTime = {}
end

-- Get bird status
function GravitySystem:isBirdAlive(entityId)
    local birdComp = Components.FlappyBird[entityId]
    return birdComp and birdComp.isAlive
end

-- Revive a bird (for game restart)
function GravitySystem:reviveBird(entityId)
    local birdComp = Components.FlappyBird[entityId]
    if birdComp then
        birdComp.isAlive = true
        birdComp.score = 0
    end
end

-- ============================================================================
-- DEBUG FUNCTIONS
-- ============================================================================

function GravitySystem:debugPrint()
    print("=== Gravity System Debug ===")
    print(string.format("  Entities with Gravity: %d", countComponents(Components.Gravity)))
    
    for entityId, gravityComp in pairs(Components.Gravity) do
        if Coordinator:HasComponent_Velocity(entityId) then
            local velocity = Coordinator:GetComponent_Velocity(entityId)
            local position = Coordinator:GetComponent_Position(entityId)
            print(string.format("  Entity %d: pos=(%.1f, %.1f), vel=(%.1f, %.1f), gravity=%.1f", 
                entityId, position.x, position.y, velocity.vx, velocity.vy, gravityComp.force))
        end
    end
end

print("✅ Gravity System loaded!")
print("   - Physics: gravity, flapping, boundaries")
print("   - Death detection: ceiling, ground, pipes")

return GravitySystem
