# 🎮 R-Type Gameplay Improvements

## Overview

This document describes the complete gameplay system overhaul, making everything **data-driven through Lua configuration files**. The engine remains **100% abstract** and can be used for any shoot'em up game.

## 📁 New File Structure

```
game/assets/scripts/
├── config/
│   ├── master_config.lua       # Main loader - loads all configs
│   ├── gameplay_config.lua     # Core gameplay settings, difficulty
│   ├── weapons_config.lua      # All weapon definitions
│   ├── enemies_config.lua      # All enemy types
│   ├── bosses_config.lua       # Boss configurations (multi-phase)
│   ├── powerups_config.lua     # Power-ups and attachments
│   └── stages_config.lua       # Stages, waves, and level design
```

## 🔧 Configuration System

### Gameplay Config (`gameplay_config.lua`)
- Player stats (health, speed, starting weapon)
- Difficulty levels (Easy, Normal, Hard, Insane)
- Screen/play area bounds
- Scoring system with combos
- Combat collision layers

### Weapons Config (`weapons_config.lua`)
All weapons fully configurable:
- **single_shot**: Basic rapid-fire with charge capability
- **double_shot**: Twin parallel shots
- **spread_shot**: Wide arc firing pattern
- **laser**: Penetrating beam (pierces enemies)
- **homing_missile**: Seeks out enemies
- **wave_beam**: Bounces off walls

Each weapon has:
- Fire rate, damage, projectile speed
- Charge levels with increasing power
- Upgrade levels (1-5)
- Projectile count and spread angle
- Special properties (piercing, homing, bouncing)

### Enemies Config (`enemies_config.lua`)
Enemy types by category:

**Common Enemies:**
- `basic` - Straight flying grunt
- `zigzag` - Zigzag movement pattern
- `sinewave` - Smooth sine wave motion
- `kamikaze` - Charges at player

**Medium Enemies:**
- `shooter` - Fires at player
- `spreader` - Fires spread shots
- `armored` - High HP, takes reduced damage

**Elite Enemies:**
- `turret` - Stationary defense gun
- `elite_fighter` - Evasive maneuvers
- `formation_leader` - Spawns minions

**Special Enemies:**
- `carrier` - Always drops power-ups
- `shielded` - Protected by regenerating shield

### Bosses Config (`bosses_config.lua`)
Multi-phase boss system:

**Stage 1 Boss: Dobkeratops**
- 3 phases with different attacks
- Weak point system
- Attacks: spread_shot, aimed_shot, laser_sweep, bullet_hell

**Stage 2 Boss: Gomander**
- Snake-like multi-segment boss
- Destroyable segments
- Spawns parasites

**Stage 3 Boss: Battleship Green**
- Multiple turrets (destroyable)
- Bridge weak point
- Missile salvos

### Power-ups Config (`powerups_config.lua`)
Collectible items:
- `speed_boost` - Increase movement speed
- `weapon_upgrade` - Level up current weapon
- `damage_boost` - Temporary damage increase
- `health_restore` - Heal player
- `shield` - Temporary protection
- `extra_life` - 1UP
- `bomb` - Screen-clearing bomb

Weapon changers:
- `laser_weapon`, `spread_weapon`, `homing_weapon`, `wave_weapon`

### Attachments
**Force Pod** (R-Type iconic):
- Attaches to front/back of ship
- Can be launched and recalled
- 3 upgrade levels
- Blocks enemy bullets

**Options** (Gradius-style):
- Up to 4 trailing satellites
- Multiple formations: trail, spread, rotate, fixed
- Mirror player fire

### Stages Config (`stages_config.lua`)
Complete level design:

**Stage 1: Space Colony**
- Wave 1: First Contact (introduction)
- Wave 2: Pressure (increased enemies)
- Wave 3: Elite Squad (elite enemies)
- Wave 4: BOSS - Dobkeratops

**Stage 2: Asteroid Belt**
- Harder enemy configurations
- New enemy combinations
- BOSS: Gomander

**Stage 3: Warship Assault**
- Heavy turret presence
- Multiple elite enemies
- BOSS: Battleship Green

## 🏗️ Engine Systems (Abstract)

All new systems in `engine/modules/shootemup/`:

### WaveSystem
- Manages stage/wave progression
- Handles enemy spawning from config
- Wave completion tracking
- Boss wave triggering

### BossSystem
- Multi-phase boss management
- Attack pattern execution
- Weak point handling
- Death sequences

### BossPartSystem
- Manages destroyable boss parts
- Part respawning
- Individual part attacks

### ForcePodSystem
- Force attachment/detachment
- Launch and recall mechanics
- Level-based firing patterns

### OptionSystem
- Trailing satellite management
- Multiple formation types
- Position history tracking

### ShieldSystem
- Temporary protection
- Hit point tracking
- Visual effects

## 🔌 Lua Integration

### GameAPI Functions
```lua
-- Get configs
GameAPI.GetEnemyConfig(enemyType)
GameAPI.GetWeaponConfig(weaponName, level)
GameAPI.GetBossConfig(bossType)
GameAPI.GetPowerUpConfig(powerUpType)
GameAPI.GetStageConfig(stageNumber)

-- Game state
GameAPI.SetDifficulty("hard")
GameAPI.GetCurrentSpawns(stageNumber, time)
GameAPI.GetActiveWaveInfo(stageNumber, time)
```

### SpawnManager
```lua
SpawnManager.StartStage(1)
local spawns = SpawnManager.Update(deltaTime)
local bossType = SpawnManager.ShouldSpawnBoss()
```

### From C++
```cpp
// Load all Lua configs
GameplayBindings::LoadMasterConfig(lua, "game/assets/scripts/");

// Get enemy config
auto config = GameplayBindings::GetEnemyConfig(lua, "zigzag");

// Start a stage
GameplayBindings::StartStage(lua, 1);

// Update spawns each frame
auto spawns = GameplayBindings::UpdateSpawns(lua, deltaTime);
for (const auto& spawn : spawns) {
    CreateEnemy(spawn.subType, spawn.x, spawn.y, spawn.pattern);
}
```

## 📝 Adding New Content

### Add New Enemy Type
Edit `enemies_config.lua`:
```lua
EnemiesConfig.my_new_enemy = {
    name = "My Enemy",
    health = 50,
    damage = 15,
    speed = 250,
    scoreValue = 300,
    movement = {
        pattern = "zigzag",
        amplitude = 100,
        frequency = 2.0
    },
    sprite = {
        texture = "enemies/my_enemy.png",
        frameWidth = 32,
        frameHeight = 32,
        scale = 2.0
    },
    animation = {
        frameCount = 4,
        frameTime = 0.1,
        loop = true
    },
    weapon = "enemy_bullet",
    shootInterval = 2.0,
    dropChance = 0.15,
    dropTable = { "weapon_upgrade", "health_restore" }
}
```

### Add New Boss
Edit `bosses_config.lua`:
```lua
BossesConfig.stage4_boss = {
    name = "New Boss",
    health = 1000,
    phases = {
        { healthThreshold = 1.0, attacks = {"spread"} },
        { healthThreshold = 0.5, attacks = {"bullet_hell"} },
        { healthThreshold = 0.25, attacks = {"rage_attack"} }
    },
    -- ... rest of config
}
```

### Add New Wave
Edit `stages_config.lua`:
```lua
-- In stage waves table
{
    name = "My New Wave",
    startTime = 60.0,
    duration = 30.0,
    spawns = {
        { time = 0.0, enemy = "my_new_enemy", y = 400, pattern = "zigzag" },
        { time = 5.0, enemy = "shooter", y = 300, pattern = "straight" },
        -- ...
    },
    reward = { type = "weapon_upgrade", y = 500 }
}
```

## 🎯 Design Philosophy

1. **Data-Driven**: All gameplay defined in Lua, no hardcoded values
2. **Engine Abstraction**: Engine systems know nothing about R-Type specifically
3. **Extensibility**: Easy to add new enemies, weapons, bosses, levels
4. **Moddability**: Players could create their own content by editing Lua
5. **Reusability**: The same engine could power a different shoot'em up game

## 🔄 Migration Guide

To use these new systems in your game loop:

```cpp
// 1. Load Lua configs at startup
GameplayBindings::LoadMasterConfig(luaState, "game/assets/scripts/");
GameplayBindings::SetDifficulty(luaState, "normal");

// 2. Initialize systems
auto waveSystem = coordinator.RegisterSystem<WaveSystem>(&coordinator);
auto bossSystem = coordinator.RegisterSystem<BossSystem>(&coordinator);
auto forceSystem = coordinator.RegisterSystem<ForcePodSystem>(&coordinator);
auto optionSystem = coordinator.RegisterSystem<OptionSystem>(&coordinator);

// 3. Start stage
GameplayBindings::StartStage(luaState, 1);

// 4. In game loop
auto spawns = GameplayBindings::UpdateSpawns(luaState, deltaTime);
for (const auto& spawn : spawns) {
    if (spawn.entityType == "enemy") {
        CreateEnemyFromConfig(spawn.subType, spawn.x, spawn.y, spawn.pattern);
    }
}

// Check for boss
std::string bossType = GameplayBindings::CheckBossSpawn(luaState);
if (!bossType.empty()) {
    auto bossConfig = GameplayBindings::GetBossConfig(luaState, bossType);
    ECS::Entity boss = CreateBoss(bossConfig);
    bossSystem->SpawnBoss(boss);
}

// Update systems
waveSystem->Update(deltaTime);
bossSystem->Update(deltaTime);
forceSystem->Update(deltaTime);
optionSystem->Update(deltaTime);
```

## 📜 License

All code and configurations are part of the R-Type project.
