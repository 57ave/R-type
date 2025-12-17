-- assets/scripts/waves/stage1_wave1.lua
-- First wave of stage 1

Wave = {
    name = "Stage 1 - Wave 1",
    duration = 30.0,
    
    -- Enemy spawn configuration
    enemies = {
        { type = "BasicEnemy", time = 1.0, x = 800, y = 100 },
        { type = "BasicEnemy", time = 2.0, x = 800, y = 200 },
        { type = "BasicEnemy", time = 2.5, x = 800, y = 300 },
        { type = "ZigzagEnemy", time = 3.5, x = 800, y = 150 },
        { type = "BasicEnemy", time = 5.0, x = 800, y = 250 },
        { type = "ZigzagEnemy", time = 6.0, x = 800, y = 350 },
        { type = "BasicEnemy", time = 8.0, x = 800, y = 100 },
        { type = "BasicEnemy", time = 8.5, x = 800, y = 200 },
        { type = "BasicEnemy", time = 9.0, x = 800, y = 300 },
    },
    
    -- Called when wave starts
    onStart = function()
        print("Wave 1 started!")
    end,
    
    -- Called when wave ends
    onComplete = function()
        print("Wave 1 complete!")
    end,
    
    -- Custom spawn logic (optional)
    -- Called every frame with current time
    customSpawn = function(time, spawner)
        -- Continuous spawning between 15-25 seconds
        if time > 15.0 and time < 25.0 then
            -- Spawn enemy every 0.5 seconds
            if time % 0.5 < 0.016 then  -- dt is typically 0.016 at 60fps
                local y = 100 + math.random() * 400
                spawner.SpawnEnemy("BasicEnemy", 800, y)
            end
        end
    end
}
