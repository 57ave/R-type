-- ============================================================================
-- FLAPPY BIRD - PIPE ENTITY FACTORY
-- ============================================================================
-- Creates pipe obstacle entities (top and bottom pairs).
-- Pipes are the main obstacles in Flappy Bird.
-- ============================================================================

print("Loading Pipe Entity Factory...")

-- Pipe configuration
PipeConfig = {
    width = 80,           -- Width of pipe sprite
    speed = -200,         -- Horizontal speed (negative = left)
    spawnX = 1280,        -- Spawn position (right edge of screen)
    destroyX = -100,      -- Destroy when off-screen
    gapHeight = 200,      -- Default gap between top and bottom pipes
    minGapY = 150,        -- Minimum gap center position
    maxGapY = 570,        -- Maximum gap center position (720 - 150)
}

-- ============================================================================
-- PIPE CREATION FUNCTIONS
-- ============================================================================

-- Create a single pipe entity (top or bottom)
-- @param x: X position
-- @param y: Y position
-- @param height: Height of the pipe
-- @param isTop: true if this is a top pipe (flipped)
-- @return entityId
function createPipeEntity(x, y, height, isTop)
    -- Create ECS entity
    local entity = Coordinator:CreateEntity()
    
    -- Position component
    Coordinator:AddComponent_Position(entity)
    local position = Coordinator:GetComponent_Position(entity)
    position.x = x
    position.y = y
    
    -- Velocity component (constant leftward movement)
    Coordinator:AddComponent_Velocity(entity)
    local velocity = Coordinator:GetComponent_Velocity(entity)
    velocity.vx = PipeConfig.speed
    velocity.vy = 0
    
    -- Collider component (for collision detection)
    Coordinator:AddComponent_Collider(entity)
    local collider = Coordinator:GetComponent_Collider(entity)
    collider.width = PipeConfig.width
    collider.height = height
    collider.offsetX = 0
    collider.offsetY = 0
    collider.enabled = true
    collider.tag = "pipe"
    
    -- Tag component
    Coordinator:AddComponent_Tag(entity)
    local tag = Coordinator:GetComponent_Tag(entity)
    tag.name = isTop and "pipe_top" or "pipe_bottom"
    
    -- Note: Sprite will be created by the rendering system
    -- We'll add sprite setup here when textures are loaded
    
    print(string.format("  Created %s pipe entity %d at (%.1f, %.1f), height: %.1f", 
        isTop and "TOP" or "BOTTOM", entity, x, y, height))
    
    return entity
end

-- Create a pair of pipes (top + bottom) with a gap in the middle
-- @param gapY: Y position of the gap center
-- @param gapHeight: Height of the gap (default: PipeConfig.gapHeight)
-- @return topEntityId, bottomEntityId, pairId (for tracking)
function createPipePair(gapY, gapHeight)
    gapHeight = gapHeight or PipeConfig.gapHeight
    
    -- Calculate pipe heights
    local topPipeHeight = gapY - (gapHeight / 2)
    local bottomPipeY = gapY + (gapHeight / 2)
    local bottomPipeHeight = 720 - bottomPipeY  -- Extend to bottom of screen
    
    -- Clamp values to ensure valid pipes
    if topPipeHeight < 50 then
        topPipeHeight = 50
        gapY = topPipeHeight + (gapHeight / 2)
        bottomPipeY = gapY + (gapHeight / 2)
        bottomPipeHeight = 720 - bottomPipeY
    end
    
    if bottomPipeHeight < 50 then
        bottomPipeHeight = 50
        bottomPipeY = 720 - bottomPipeHeight
        gapY = bottomPipeY - (gapHeight / 2)
        topPipeHeight = gapY - (gapHeight / 2)
    end
    
    -- Create top pipe (hangs from ceiling)
    local topEntity = createPipeEntity(
        PipeConfig.spawnX,
        0,  -- Starts at top of screen
        topPipeHeight,
        true  -- isTop
    )
    
    -- Create bottom pipe (rises from ground)
    local bottomEntity = createPipeEntity(
        PipeConfig.spawnX,
        bottomPipeY,
        bottomPipeHeight,
        false  -- isTop
    )
    
    -- Generate unique pair ID (use top entity ID as identifier)
    local pairId = topEntity
    
    -- Create Lua Pipe component for both (shared data)
    local pipeData = createPipeComponent(topEntity, gapY, gapHeight)
    Components.Pipe[bottomEntity] = pipeData  -- Share the same component
    
    print(string.format("Created pipe pair %d: gap Y=%.1f, height=%.1f", 
        pairId, gapY, gapHeight))
    
    return topEntity, bottomEntity, pairId
end

-- Create a random pipe pair
-- @return topEntityId, bottomEntityId, pairId
function createRandomPipePair()
    -- Random gap position
    local gapY = math.random(PipeConfig.minGapY, PipeConfig.maxGapY)
    
    -- Optional: vary gap height slightly for difficulty
    local gapHeight = PipeConfig.gapHeight + math.random(-20, 20)
    
    return createPipePair(gapY, gapHeight)
end

-- ============================================================================
-- PIPE HELPER FUNCTIONS
-- ============================================================================

-- Check if a pipe is off-screen (should be destroyed)
-- @param entityId: the pipe entity
-- @return true if off-screen
function isPipeOffScreen(entityId)
    if not Coordinator:HasComponent_Position(entityId) then
        return false
    end
    
    local position = Coordinator:GetComponent_Position(entityId)
    return position.x < PipeConfig.destroyX
end

-- Check if a pipe has been passed by a bird
-- @param pipeEntityId: the pipe entity
-- @param birdEntityId: the bird entity
-- @return true if bird passed the pipe
function hasBirdPassedPipe(pipeEntityId, birdEntityId)
    if not Coordinator:HasComponent_Position(pipeEntityId) or 
       not Coordinator:HasComponent_Position(birdEntityId) then
        return false
    end
    
    local pipePos = Coordinator:GetComponent_Position(pipeEntityId)
    local birdPos = Coordinator:GetComponent_Position(birdEntityId)
    
    -- Bird has passed if its X is greater than pipe's X + width
    return birdPos.x > (pipePos.x + PipeConfig.width)
end

-- Get all pipe entities
-- @return table of entity IDs
function getAllPipes()
    local pipes = {}
    
    for entityId, _ in pairs(Components.Pipe) do
        table.insert(pipes, entityId)
    end
    
    return pipes
end

-- Destroy a pipe entity
-- @param entityId: the pipe to destroy
function destroyPipe(entityId)
    -- Remove Lua components
    removeAllComponents(entityId)
    
    -- Destroy ECS entity
    Coordinator:DestroyEntity(entityId)
    
    print(string.format("  Destroyed pipe entity %d", entityId))
end

print("Pipe Entity Factory loaded!")
print(string.format("   - Pipe width: %d, speed: %d", PipeConfig.width, PipeConfig.speed))
print(string.format("   - Gap height: %d, Y range: %d-%d", 
    PipeConfig.gapHeight, PipeConfig.minGapY, PipeConfig.maxGapY))

return {
    createPipePair = createPipePair,
    createRandomPipePair = createRandomPipePair,
    isPipeOffScreen = isPipeOffScreen,
    hasBirdPassedPipe = hasBirdPassedPipe,
    getAllPipes = getAllPipes,
    destroyPipe = destroyPipe,
    config = PipeConfig
}
