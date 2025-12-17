-- assets/scripts/systems/enemy_ai_system.lua
-- Enemy AI behavior system

-- Update function called every frame
-- entities: table of entity IDs that match the system signature
-- dt: delta time in seconds
-- coordinator: reference to ECS coordinator

function update(entities, dt, coordinator)
    for i = 1, #entities do
        local entity = entities[i]
        
        -- Get components
        local transform = Coordinator.GetTransform(entity)
        local velocity = Coordinator.GetVelocity(entity)
        local ai = Coordinator.GetAIController(entity)
        
        -- Update AI timer
        ai.timer = ai.timer + dt
        
        -- Different movement patterns
        if ai.pattern == "straight" then
            -- Simple straight movement (velocity already set)
            
        elseif ai.pattern == "zigzag" then
            -- Sine wave vertical movement
            velocity.dy = math.sin(ai.timer * 2) * 150
            
        elseif ai.pattern == "circle" then
            -- Circular movement
            local radius = ai.circleRadius or 100
            transform.x = ai.centerX + math.cos(ai.timer) * radius
            transform.y = ai.centerY + math.sin(ai.timer) * radius
            
        elseif ai.pattern == "dive" then
            -- Dive down then return up
            if transform.y < ai.targetY then
                velocity.dy = 300
            else
                velocity.dy = -200
                ai.targetY = 100 + math.random() * 400
            end
        end
        
        -- Shooting logic
        ai.shootTimer = ai.shootTimer + dt
        if ai.shootTimer >= ai.shootInterval then
            ai.shootTimer = 0
            -- TODO: Call C++ function to spawn bullet
            -- SpawnBullet(transform.x, transform.y, -300, 0)
        end
        
        -- Destroy if off-screen (left side)
        if transform.x < -50 then
            Coordinator.DestroyEntity(entity)
        end
    end
end
