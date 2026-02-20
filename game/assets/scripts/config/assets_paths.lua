-- ==========================================
-- R-Type Game - Assets Paths Configuration
-- ==========================================

Assets = {
    base_path = "assets/",
    
    -- Textures des joueurs
    players = {
        player_ship = "players/player_ship.png",
        player_spritesheet = "players/player_animations.png"
    },
    
    -- Textures des ennemis
    enemies = {
        basic = "enemies/enemy_basic.png",
        fast = "enemies/enemy_fast.png",
        tank = "enemies/enemy_tank.png",
        kamikaze = "enemies/enemy_kamikaze.png",
        sniper = "enemies/enemy_sniper.png"
    },
    
    -- Textures des boss
    bosses = {
        guardian = "enemies/boss_guardian.png",
        destroyer = "enemies/boss_destroyer.png"
    },
    
    -- Projectiles
    projectiles = {
        basic = "vfx/projectile_basic.png",
        charge = "vfx/projectile_charge.png",
        laser = "vfx/laser_beam.png",
        missile = "vfx/missile.png",
        spread = "vfx/projectile_spread.png"
    },
    
    -- VFX et particules
    vfx = {
        explosion_small = "vfx/explosion_small.png",
        explosion_medium = "vfx/explosion_medium.png",
        explosion_large = "vfx/explosion_large.png",
        hit_effect = "vfx/hit_spark.png",
        shield_effect = "vfx/shield.png",
        powerup_glow = "vfx/powerup_glow.png",
        particle_trail = "vfx/trail_particle.png"
    },
    
    -- Backgrounds et parallax
    backgrounds = {
        layer_1 = "backgrounds/space_bg_1.png",
        layer_2 = "backgrounds/space_bg_2.png",
        layer_3 = "backgrounds/space_bg_3.png",
        layer_4 = "backgrounds/stars.png",
        nebula = "backgrounds/nebula.png"
    },
    
    -- UI
    ui = {
        button_normal = "ui/button_normal.png",
        button_hover = "ui/button_hover.png",
        button_pressed = "ui/button_pressed.png",
        panel = "ui/panel.png",
        health_bar = "ui/health_bar.png",
        energy_bar = "ui/energy_bar.png",
        cursor = "ui/cursor.png",
        icons = "ui/icons_spritesheet.png"
    },
    
    -- Fonts
    fonts = {
        main = "fonts/main_font.ttf",
        title = "fonts/title_font.ttf",
        mono = "fonts/mono_font.ttf"
    },
    
    -- Sons (SFX)
    sounds = {
        -- Joueur
        shoot_basic = "sounds/player_shoot.ogg",
        shoot_charge = "sounds/player_charge_shot.ogg",
        laser_loop = "sounds/laser_loop.ogg",
        missile_launch = "sounds/missile_launch.ogg",
        player_hit = "sounds/player_hit.ogg",
        player_death = "sounds/player_death.ogg",
        
        -- Ennemis
        enemy_spawn = "sounds/enemy_spawn.ogg",
        enemy_shoot = "sounds/enemy_shoot.ogg",
        enemy_death = "sounds/enemy_death.ogg",
        enemy_death_fast = "sounds/enemy_death_fast.ogg",
        enemy_death_explosion = "sounds/explosion_big.ogg",
        
        -- Boss
        boss_guardian_spawn = "sounds/boss_spawn.ogg",
        boss_destroyer_spawn = "sounds/boss_spawn_heavy.ogg",
        boss_attack = "sounds/boss_attack.ogg",
        boss_hurt = "sounds/boss_hurt.ogg",
        boss_death_explosion = "sounds/boss_death.ogg",
        boss_phase_change = "sounds/phase_change.ogg",
        
        -- UI
        button_click = "sounds/ui_click.ogg",
        button_hover = "sounds/ui_hover.ogg",
        menu_select = "sounds/menu_select.ogg",
        menu_back = "sounds/menu_back.ogg",
        
        -- Gameplay
        powerup_pickup = "sounds/powerup.ogg",
        explosion_small = "sounds/explosion_small.ogg",
        explosion_medium = "sounds/explosion_medium.ogg",
        laser_fire = "sounds/laser_fire.ogg",
        laser_charge = "sounds/laser_charge.ogg",
        kamikaze_alert = "sounds/kamikaze_alert.ogg",
        kamikaze_explosion = "sounds/kamikaze_boom.ogg"
    },
    
    -- Musiques
    music = {
        main_menu = "sounds/music_menu.ogg",
        level_1 = "sounds/music_level1.ogg",
        level_2 = "sounds/music_level2.ogg",
        boss_guardian_theme = "sounds/music_boss1.ogg",
        boss_destroyer_theme = "sounds/music_boss2.ogg",
        victory = "sounds/music_victory.ogg",
        game_over = "sounds/music_gameover.ogg"
    }
}



return Assets
