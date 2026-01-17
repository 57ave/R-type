-- assets/scripts/systems/spawn_system.lua
-- Minimal spawn system to avoid missing-script error.
-- Defines update(entities, dt, coordinator) which the engine will call.

local spawnTimer = 0.0
local spawnInterval = 2.0 -- seconds between spawns (tweakable)

function update(entities, dt, coordinator)
    spawnTimer = spawnTimer + dt
    if spawnTimer >= spawnInterval then
        spawnTimer = spawnTimer - spawnInterval

        -- Simple spawn position: slightly off the right edge, random y
        local x = 1920 + 50
        local y = math.random(50, 900)

        if Factory and Factory.CreateEnemy then
            -- Use the generic CreateEnemy factory: first arg is type string
            -- Replace "basic" with a real enemy type from your configs if needed
            Factory.CreateEnemy("basic", x, y)
            print("[SpawnSystem] Spawned enemy 'basic' at", x, y)
        else
            print("[SpawnSystem] Factory.CreateEnemy not available in Lua state")
        end
    end
end
