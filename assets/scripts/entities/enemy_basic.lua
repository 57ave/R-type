-- assets/scripts/entities/enemy_basic.lua
-- Basic enemy prefab

prefab = {
    components = {
        Transform = {
            x = 800,
            y = 300,
            rotation = 0
        },
        
        Velocity = {
            dx = -150,
            dy = 0,
            maxSpeed = 200
        },
        
        Sprite = {
            texture = "assets/textures/enemy_basic.png",
            width = 32,
            height = 32
        },
        
        Health = {
            current = 30,
            max = 30
        },
        
        Damage = {
            value = 10
        },
        
        AIController = {
            pattern = "straight",
            shootInterval = 2.0
        },
        
        Collider = {
            radius = 16,
            isTrigger = false
        }
    },
    
    -- Optional: Called when entity is spawned
    onSpawn = function(entity)
        print("Basic enemy spawned at entity ID: " .. tostring(entity))
    end
}
