-- assets/scripts/entities/enemy_zigzag.lua
-- Enemy with zigzag movement pattern

prefab = {
    components = {
        Transform = {
            x = 800,
            y = 300,
            rotation = 0
        },
        
        Velocity = {
            dx = -120,
            dy = 0,
            maxSpeed = 250
        },
        
        Sprite = {
            texture = "assets/textures/enemy_zigzag.png",
            width = 32,
            height = 32
        },
        
        Health = {
            current = 50,
            max = 50
        },
        
        Damage = {
            value = 15
        },
        
        AIController = {
            pattern = "zigzag",
            shootInterval = 1.5,
            timer = 0
        },
        
        Collider = {
            radius = 16,
            isTrigger = false
        }
    }
}
