-- ============================================================================
-- FLAPPY BIRD - LUA COMPONENT SYSTEM
-- ============================================================================
-- This file defines all Lua-side components for the Flappy Bird game.
-- These components are stored in tables indexed by entity ID.
-- Pattern: Components.ComponentName[entityId] = { ... }
-- ============================================================================

print("📦 Loading Flappy Bird Lua Components...")

-- Component storage tables
Components = {
    FlappyBird = {},    -- Bird-specific data
    Gravity = {},       -- Gravity physics
    Pipe = {},          -- Pipe obstacles
    Score = {},         -- Score tracking
    GameState = {}      -- Global game state (singleton, stored at index 0)
}

-- ============================================================================
-- COMPONENT DEFINITIONS
-- ============================================================================

-- FlappyBird Component
-- Stores bird-specific game data (player ID, state, appearance, physics)
-- Fields:
--   playerId: unique ID for this player (0-99)
--   isAlive: whether the bird is still in the game
--   color: visual color/skin (0-3 for different bird sprites)
--   flapStrength: upward velocity applied on flap
--   score: current score for this bird
function Components.FlappyBird:new(playerId, isAlive, color, flapStrength, score)
    return {
        playerId = playerId or 0,
        isAlive = isAlive ~= false,  -- default true
        color = color or 0,
        flapStrength = flapStrength or -400.0,  -- negative = upward
        score = score or 0
    }
end

-- Gravity Component
-- Applies constant downward acceleration
-- Fields:
--   force: downward acceleration (pixels/s²)
--   terminalVelocity: maximum falling speed (pixels/s)
function Components.Gravity:new(force, terminalVelocity)
    return {
        force = force or 1200.0,              -- gravity acceleration
        terminalVelocity = terminalVelocity or 800.0  -- max fall speed
    }
end

-- Pipe Component
-- Obstacle pipe data
-- Fields:
--   gapY: Y position of the gap center
--   gapHeight: height of the gap (pixels)
--   passed: has any bird passed through this pipe?
--   scored: table of player IDs who scored from this pipe
function Components.Pipe:new(gapY, gapHeight)
    return {
        gapY = gapY or 360.0,
        gapHeight = gapHeight or 200.0,
        passed = false,
        scored = {}  -- list of player IDs who scored
    }
end

-- Score Component
-- Individual player score tracking
-- Fields:
--   value: current score value
--   playerId: which player this score belongs to
function Components.Score:new(playerId, value)
    return {
        value = value or 0,
        playerId = playerId or 0
    }
end

-- GameState Component (Singleton)
-- Global game state - stored at index 0
-- Fields:
--   state: current game state ("waiting", "countdown", "playing", "gameover")
--   countdown: countdown timer (seconds)
--   winnerId: ID of the winning player (-1 = none)
--   startTime: when the game started (for total time)
--   alivePlayers: count of alive players
function Components.GameState:new(state, countdown, winnerId)
    return {
        state = state or "waiting",
        countdown = countdown or 3.0,
        winnerId = winnerId or -1,
        startTime = 0.0,
        alivePlayers = 0
    }
end

-- ============================================================================
-- FACTORY FUNCTIONS
-- ============================================================================

-- Create a FlappyBird component for an entity
-- @param entityId: ECS entity ID
-- @param playerId: unique player ID (0-99)
-- @param color: bird color/sprite variant (0-3)
-- @return the created component
function createBirdComponent(entityId, playerId, color)
    local component = Components.FlappyBird:new(
        playerId,
        true,       -- isAlive
        color,
        -400.0,     -- flapStrength (upward velocity)
        0           -- initial score
    )
    Components.FlappyBird[entityId] = component
    print(string.format("  🐦 Created FlappyBird component for entity %d (Player %d, Color %d)", 
        entityId, playerId, color))
    return component
end

-- Create a Gravity component for an entity
-- @param entityId: ECS entity ID
-- @param force: gravity force (default: 1200.0)
-- @param terminalVelocity: max fall speed (default: 800.0)
-- @return the created component
function createGravityComponent(entityId, force, terminalVelocity)
    local component = Components.Gravity:new(force, terminalVelocity)
    Components.Gravity[entityId] = component
    print(string.format("  ⬇️  Created Gravity component for entity %d (force: %.1f)", 
        entityId, component.force))
    return component
end

-- Create a Pipe component for an entity
-- @param entityId: ECS entity ID
-- @param gapY: Y position of gap center
-- @param gapHeight: height of the gap in pixels
-- @return the created component
function createPipeComponent(entityId, gapY, gapHeight)
    local component = Components.Pipe:new(gapY, gapHeight)
    Components.Pipe[entityId] = component
    print(string.format("  🚧 Created Pipe component for entity %d (gap Y: %.1f, height: %.1f)", 
        entityId, gapY, gapHeight))
    return component
end

-- Create a Score component for an entity
-- @param entityId: ECS entity ID
-- @param playerId: player ID this score belongs to
-- @return the created component
function createScoreComponent(entityId, playerId)
    local component = Components.Score:new(playerId, 0)
    Components.Score[entityId] = component
    print(string.format("  🏆 Created Score component for entity %d (Player %d)", 
        entityId, playerId))
    return component
end

-- Create the global GameState (singleton at index 0)
-- @return the created component
function createGameStateComponent()
    local component = Components.GameState:new("waiting", 3.0, -1)
    Components.GameState[0] = component
    print("  🎮 Created GameState component (singleton)")
    return component
end

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Get a component for an entity (with nil check)
-- @param componentTable: the component storage table (e.g., Components.FlappyBird)
-- @param entityId: the entity ID
-- @return the component or nil
function getComponent(componentTable, entityId)
    return componentTable[entityId]
end

-- Check if an entity has a component
-- @param componentTable: the component storage table
-- @param entityId: the entity ID
-- @return true if the component exists
function hasComponent(componentTable, entityId)
    return componentTable[entityId] ~= nil
end

-- Remove a component from an entity
-- @param componentTable: the component storage table
-- @param entityId: the entity ID
function removeComponent(componentTable, entityId)
    componentTable[entityId] = nil
end

-- Remove all components for an entity (cleanup)
-- @param entityId: the entity ID to clean up
function removeAllComponents(entityId)
    Components.FlappyBird[entityId] = nil
    Components.Gravity[entityId] = nil
    Components.Pipe[entityId] = nil
    Components.Score[entityId] = nil
    -- Note: GameState is singleton, don't remove
end

-- Count entities with a specific component
-- @param componentTable: the component storage table
-- @return count of entities with this component
function countComponents(componentTable)
    local count = 0
    for _ in pairs(componentTable) do
        count = count + 1
    end
    return count
end

-- Iterate over all entities with a component
-- @param componentTable: the component storage table
-- @param callback: function(entityId, component) to call for each
function forEachComponent(componentTable, callback)
    for entityId, component in pairs(componentTable) do
        callback(entityId, component)
    end
end

-- ============================================================================
-- DEBUG FUNCTIONS
-- ============================================================================

-- Print all components for an entity
function debugPrintEntity(entityId)
    print(string.format("=== Entity %d Components ===", entityId))
    
    if Components.FlappyBird[entityId] then
        local bird = Components.FlappyBird[entityId]
        print(string.format("  FlappyBird: Player %d, Alive: %s, Score: %d", 
            bird.playerId, tostring(bird.isAlive), bird.score))
    end
    
    if Components.Gravity[entityId] then
        local grav = Components.Gravity[entityId]
        print(string.format("  Gravity: Force %.1f, Terminal %.1f", 
            grav.force, grav.terminalVelocity))
    end
    
    if Components.Pipe[entityId] then
        local pipe = Components.Pipe[entityId]
        print(string.format("  Pipe: Gap Y %.1f, Height %.1f, Passed: %s", 
            pipe.gapY, pipe.gapHeight, tostring(pipe.passed)))
    end
    
    if Components.Score[entityId] then
        local score = Components.Score[entityId]
        print(string.format("  Score: Value %d, Player %d", 
            score.value, score.playerId))
    end
end

-- Print summary of all components
function debugPrintComponentSummary()
    print("=== Component Summary ===")
    print(string.format("  FlappyBird: %d", countComponents(Components.FlappyBird)))
    print(string.format("  Gravity: %d", countComponents(Components.Gravity)))
    print(string.format("  Pipe: %d", countComponents(Components.Pipe)))
    print(string.format("  Score: %d", countComponents(Components.Score)))
    
    if Components.GameState[0] then
        local gs = Components.GameState[0]
        print(string.format("  GameState: %s (Alive: %d)", gs.state, gs.alivePlayers))
    end
end

print("✅ Flappy Bird Lua Components loaded!")
print("   - FlappyBird, Gravity, Pipe, Score, GameState")
print("   - Factory functions ready")

return Components
