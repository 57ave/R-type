# Flappy Bird - Game 2

## Description
This is a Flappy Bird clone built with the R-Type engine.

## Project Structure
```
game2/
├── CMakeLists.txt      # Build configuration
├── main.cpp            # Entry point
├── include/            # Header files (.hpp)
├── src/                # Implementation files (.cpp)
├── assets/             # Game assets (sprites, sounds, etc.)
└── README.md           # This file
```

## Building

From the project root:
```bash
cmake -B build -DBUILD_GAME2=ON
cmake --build build --target flappy_bird
```

## Running

From the project root:
```bash
./game2/flappy_bird
```

## Development

### Adding New Classes

1. Create header in `include/` (e.g., `include/Bird.hpp`)
2. Create implementation in `src/` (e.g., `src/Bird.cpp`)
3. CMake will automatically pick up the files

### Using the Engine

The game has access to the full R-Type engine:
- ECS system (`#include <ecs/ECS.hpp>`)
- Rendering (`#include <rendering/sfml/...>`)
- Audio (`#include <engine/Audio.hpp>`)
- Input (`#include <engine/Input.hpp>`)
- Scripting (`#include <scripting/LuaState.hpp>`)

Example:
```cpp
#include <ecs/Coordinator.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <systems/MovementSystem.hpp>
```

## Assets

Place your assets in the `assets/` folder:
- `assets/sprites/` - Sprite images
- `assets/sounds/` - Sound effects
- `assets/music/` - Background music
- `assets/fonts/` - Fonts

These will be automatically copied to the build directory.
