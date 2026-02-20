-- ==========================================
-- R-Type Game - VFX Configuration
-- ==========================================

VFX = {
    -- Enemy death explosion
    dead_enemies_animation = {
        texture_path = "assets/enemies/dead_enemies_animation.png",
        
        -- Animation frames
        animation = {
            frame_count = 6,           -- Total number of frames
            frame_width = 33,          -- Width of each frame
            frame_height = 33,         -- Height of each frame
            frame_time = 0.05,         -- Time per frame (50ms = 20 FPS)
            loop = false,              -- Play once then destroy
            start_x = 0,               -- Starting X position in spritesheet
            start_y = 0                -- Starting Y position in spritesheet
        },
        
        -- Sprite properties
        sprite = {
            scale = {2.0, 2.0},        -- Scale factors
            layer = 5                   -- Render above everything
        },
        
        -- Lifetime (auto-destroy after animation)
        lifetime = 0.30                -- 6 frames * 0.05s = 0.30s
    },
    
    -- Player death explosion (if needed later)
    player_explosion = {
        texture_path = "assets/vfx/player_explosion.png",
        
        animation = {
            frame_count = 8,
            frame_width = 64,
            frame_height = 64,
            frame_time = 0.08,
            loop = false,
            start_x = 0,
            start_y = 0
        },
        
        sprite = {
            scale = {2.5, 2.5},
            layer = 5
        },
        
        lifetime = 0.64                -- 8 * 0.08
    },
    
    -- Hit spark (small impact effect)
    hit_spark = {
        texture_path = "assets/vfx/hit_spark.png",
        
        animation = {
            frame_count = 4,
            frame_width = 16,
            frame_height = 16,
            frame_time = 0.04,
            loop = false,
            start_x = 0,
            start_y = 0
        },
        
        sprite = {
            scale = {2.0, 2.0},
            layer = 4
        },
        
        lifetime = 0.16                -- 4 * 0.04
    }
}



return VFX
