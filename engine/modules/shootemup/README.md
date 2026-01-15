# Shoot'em Up Module for R-Type Engine

This module provides **reusable components, systems, and utilities** for creating shoot'em up games.

## 🎮 What's included

### Components
- **Weapon** - Weapon types, firing rates, projectile patterns
- **MovementPattern** - Enemy movement AI (sine wave, zigzag, circular, etc.)
- **Attachment** - Weapon attachment points (wings, turrets)
- **ShootEmUpTags** - PlayerTag, EnemyTag, ProjectileTag with shoot'em up specific data
- **PowerUp** - Collectible power-ups (speed boost, weapon upgrades, etc.)

### Systems
- **WeaponSystem** - Handles weapon firing, cooldowns, ammo
- **MovementPatternSystem** - Executes enemy movement patterns
- **EnemySpawnSystem** - Wave-based enemy spawning
- **PowerUpSystem** - Power-up collection and effects

### Factories
- **EnemyFactory** - Create different enemy types with presets
- **ProjectileFactory** - Create various projectile types
- **PowerUpFactory** - Create power-up entities

## 📦 Usage

### In your game's CMakeLists.txt:
```cmake
# Link the shoot'em up module
target_link_libraries(your_game PRIVATE 
    engine_core
    engine_module_shootemup
)
```

### In your game code:
```cpp
#include <shootemup/components/Weapon.hpp>
#include <shootemup/systems/WeaponSystem.hpp>
#include <shootemup/factories/EnemyFactory.hpp>

// Use the module's components and systems
auto weaponSystem = std::make_shared<ShootEmUp::WeaponSystem>(&coordinator);
coordinator.RegisterSystem(weaponSystem);
```

## 🔧 Customization

You can:
- Extend the provided components
- Override factory methods for custom enemies
- Create your own movement patterns
- Add new weapon types

## 📝 License

Part of the R-Type Engine project.
