# 🎮 R-Type Engine - Module Architecture

## 📂 New Architecture Overview

The R-Type Engine now follows a **modular architecture** where:
- **`engine/`** contains ONLY generic, reusable code
- **`engine/modules/`** contains optional genre-specific modules
- **`game/`** contains ONLY game-specific code and configuration

```
R-type/
├── engine/                          # ✅ 100% Generic Engine Core
│   ├── include/
│   │   ├── ecs/                    # Pure ECS (Entity Component System)
│   │   ├── components/             # GENERIC components (Transform, Velocity, Health, etc.)
│   │   ├── rendering/              # Rendering abstraction
│   │   ├── scripting/              # Lua scripting infrastructure
│   │   └── network/                # Network abstraction
│   │
│   └── modules/                    # 🎯 Optional Genre-Specific Modules
│       └── shootemup/              # Shoot'em up game module (REUSABLE)
│           ├── include/
│           │   ├── components/     # Weapon, MovementPattern, PowerUp, AIController, etc.
│           │   ├── systems/        # WeaponSystem, MovementPatternSystem, EnemySpawnSystem
│           │   └── factories/      # EnemyFactory, ProjectileFactory
│           ├── src/
│           └── CMakeLists.txt      # Independent module build
│
└── game/                            # 🎲 R-Type Game Project
    ├── include/
    │   ├── components/             # R-Type SPECIFIC components (if any)
    │   │   └── GameComponents.hpp  # Player, Enemy extensions specific to THIS R-Type
    │   └── scripting/              # R-Type specific Lua bindings
    ├── src/
    │   ├── Game.cpp                # Main game logic
    │   └── scripting/              # FactoryBindings, GameScriptBindings
    ├── assets/                     # Textures, sounds, Lua scripts
    └── CMakeLists.txt              # Links to engine + shootemup module
```

---

## 🔧 How It Works

### 1. **Engine Core** (100% Generic)
```cpp
// engine/include/components/ - ONLY universal components
Transform.hpp       // Position, rotation (ANY game needs this)
Velocity.hpp        // Movement (ANY game with physics)
Sprite.hpp          // Visual (ANY 2D game)
Health.hpp          // HP system (ANY game with health)
Collider.hpp        // Collision (ANY game with physics)
Tag.hpp             // Generic entity tagging (ANY game)
```

### 2. **Shoot'em Up Module** (Reusable for ALL shoot'em ups)
```cpp
// engine/modules/shootemup/include/components/
Weapon.hpp              // Fire rates, projectile types, ammo
MovementPattern.hpp     // AI patterns (sine wave, zigzag, circular)
Attachment.hpp          // Weapon attachment points
PowerUp.hpp             // Collectibles (speed, damage, shield, etc.)
AIController.hpp        // Enemy AI behavior
ShootEmUpTags.hpp       // PlayerTag, EnemyTag, ProjectileTag

// engine/modules/shootemup/include/systems/
WeaponSystem.hpp        // Handles firing, cooldowns
MovementPatternSystem.hpp // Executes enemy movement
EnemySpawnSystem.hpp    // Wave-based spawning

// engine/modules/shootemup/include/factories/
EnemyFactory.hpp        // Create enemies with presets
ProjectileFactory.hpp   // Create projectiles
```

### 3. **Game Project** (R-Type Specific)
```cpp
// game/include/components/
GameComponents.hpp      // R-Type SPECIFIC extensions (if needed)

// game/src/
Game.cpp                // Main game loop, level design, UI
FactoryBindings.cpp     // Lua bindings for R-Type factories
GameScriptBindings.cpp  // Lua bindings for R-Type components

// game/assets/scripts/
game_config.lua         // R-Type specific configuration
waves/*.lua             // Enemy wave definitions
```

---

## 🚀 Benefits

### ✅ For Engine Developers
- **100% Generic Core**: Engine can be used for ANY game type
- **Modular**: Add new modules (platformer, rpg, etc.) without touching core
- **Testable**: Each module is independent

### ✅ For Game Developers
- **Batteries Included**: Use shootemup module for instant shoot'em up features
- **No Reinventing the Wheel**: Enemy AI, weapons, power-ups already implemented
- **Customizable**: Can override or extend any module component

### ✅ For Community
- **Reusable Modules**: Create a new shoot'em up by just using the module
- **Contribute Modules**: Add platformer, RPG, puzzle modules
- **Share**: Other teams can use your modules

---

## 📦 Creating a New Shoot'em Up Game (5 minutes!)

```bash
# 1. Create a new game folder
mkdir my-shootemup
cd my-shootemup

# 2. Create CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.20)
project(MyShootEmUp)

add_executable(my_game main.cpp)

target_link_libraries(my_game PRIVATE
    engine_core          # Core ECS + rendering
    shootemup            # Shoot'em up module
)
EOF

# 3. Create main.cpp
cat > main.cpp << 'EOF'
#include <shootemup/factories/EnemyFactory.hpp>
#include <shootemup/systems/WeaponSystem.hpp>

int main() {
    // Your game logic using the module!
    return 0;
}
EOF

# 4. Build
cmake -B build && cmake --build build
./build/my_game
```

**Done! You have a shoot'em up game with:**
- ✅ Weapon system
- ✅ Enemy AI patterns
- ✅ Power-ups
- ✅ Factories
- ✅ All shoot'em up components

---

## 🧩 Adding New Modules

Want to create a **platformer module**?

```
engine/modules/platformer/
├── include/
│   ├── components/
│   │   ├── Jump.hpp
│   │   ├── Gravity.hpp
│   │   ├── Ladder.hpp
│   │   └── Platform.hpp
│   └── systems/
│       ├── PhysicsSystem.hpp
│       └── PlatformCollisionSystem.hpp
└── CMakeLists.txt
```

Then any game can use it:
```cmake
target_link_libraries(my_platformer PRIVATE
    engine_core
    platformer      # ← Your new module!
)
```

---

## 📚 More Information

- **Engine Core Documentation**: [`engine/README.md`](../engine/README.md)
- **Shoot'em Up Module**: [`engine/modules/shootemup/README.md`](../engine/modules/shootemup/README.md)
- **Game Project**: [`game/README.md`](../game/README.md)

---

**Built with ❤️ by the R-Type Team**
