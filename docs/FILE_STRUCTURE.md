# 📁 Structure des Fichiers - Game Engine

> Guide pour organiser les fichiers du game engine

---

## 🌳 Arborescence Complète

```
rtype/
├── CMakeLists.txt                    # Root CMake
├── README.md                         # Project README
│
├── docs/                             # 📚 Documentation
│   ├── README.md                     # Index de la documentation
│   ├── ENGINE_IMPLEMENTATION_GUIDE.md
│   ├── QUICK_REFERENCE.md
│   ├── ROADMAP.md
│   ├── FILE_STRUCTURE.md            # ← Ce fichier
│   ├── engine_architecture.puml
│   ├── ecs_detailed.puml
│   └── client_server_flow.puml
│
├── engine/                           # 🎮 Game Engine (ta partie)
│   ├── CMakeLists.txt
│   ├── README.md
│   │
│   ├── include/              # Headers publics
│   │   │
│   │   ├── ecs/                     # Entity Component System
│   │   │   ├── Entity.hpp
│   │   │   ├── EntityManager.hpp
│   │   │   ├── ComponentManager.hpp
│   │   │   ├── SparseSet.hpp
│   │   │   ├── Registry.hpp
│   │   │   ├── System.hpp
│   │   │   ├── SystemManager.hpp
│   │   │   └── View.hpp
│   │   │
│   │   ├── core/                    # Core utilities
│   │   │   ├── Time.hpp
│   │   │   ├── Logger.hpp
│   │   │   ├── EventBus.hpp
│   │   │   ├── Event.hpp
│   │   │   ├── ResourceManager.hpp
│   │   │   ├── Config.hpp
│   │   │   ├── InputManager.hpp
│   │   │   └── Types.hpp           # Vector2, Rect, etc.
│   │   │
│   │   ├── network/                 # Network abstraction
│   │   │   ├── Packet.hpp
│   │   │   ├── ISocket.hpp
│   │   │   ├── UDPSocket.hpp
│   │   │   ├── Connection.hpp
│   │   │   ├── ConnectionManager.hpp
│   │   │   ├── NetworkInterpolator.hpp
│   │   │   └── PacketTypes.hpp
│   │   │
│   │   ├── rendering/               # Graphics abstraction
│   │   │   ├── IRenderer.hpp
│   │   │   ├── ITexture.hpp
│   │   │   ├── ISprite.hpp
│   │   │   ├── Window.hpp
│   │   │   └── Camera.hpp
│   │   │
│   │   ├── physics/                 # Physics & collision
│   │   │   ├── CollisionDetector.hpp
│   │   │   ├── Collider.hpp
│   │   │   ├── QuadTree.hpp
│   │   │   └── Ray.hpp
│   │   │
│   │   └── systems/                 # Common systems
│   │       ├── MovementSystem.hpp
│   │       ├── CollisionSystem.hpp
│   │       ├── RenderSystem.hpp
│   │       ├── AnimationSystem.hpp
│   │       ├── LifetimeSystem.hpp
│   │       └── ParticleSystem.hpp
│   │
│   └── src/                         # Implementations
│       │
│       ├── ecs/
│       │   ├── EntityManager.cpp
│       │   ├── Registry.cpp
│       │   └── SystemManager.cpp
│       │
│       ├── core/
│       │   ├── Time.cpp
│       │   ├── Logger.cpp
│       │   ├── EventBus.cpp
│       │   ├── Config.cpp
│       │   └── InputManager.cpp
│       │
│       ├── network/
│       │   ├── Packet.cpp
│       │   ├── UDPSocket.cpp
│       │   ├── Connection.cpp
│       │   ├── ConnectionManager.cpp
│       │   └── NetworkInterpolator.cpp
│       │
│       ├── rendering/
│       │   ├── Window.cpp
│       │   └── Camera.cpp
│       │
│       ├── physics/
│       │   ├── CollisionDetector.cpp
│       │   └── QuadTree.cpp
│       │
│       └── systems/
│           ├── MovementSystem.cpp
│           ├── CollisionSystem.cpp
│           ├── RenderSystem.cpp
│           ├── AnimationSystem.cpp
│           ├── LifetimeSystem.cpp
│           └── ParticleSystem.cpp
│
├── game/                            # 🎯 Game Logic (R-Type specific)
│   ├── CMakeLists.txt
│   │
│   ├── include/game/
│   │   ├── components/              # Game-specific components
│   │   │   ├── Player.hpp
│   │   │   ├── Enemy.hpp
│   │   │   ├── Bullet.hpp
│   │   │   ├── PowerUp.hpp
│   │   │   └── Starfield.hpp
│   │   │
│   │   └── systems/                 # Game-specific systems
│   │       ├── PlayerControlSystem.hpp
│   │       ├── EnemyAISystem.hpp
│   │       ├── WeaponSystem.hpp
│   │       ├── ScoreSystem.hpp
│   │       └── SpawnSystem.hpp
│   │
│   └── src/
│       ├── components/
│       └── systems/
│           ├── PlayerControlSystem.cpp
│           ├── EnemyAISystem.cpp
│           ├── WeaponSystem.cpp
│           ├── ScoreSystem.cpp
│           └── SpawnSystem.cpp
│
├── client/                          # 💻 Client Application
│   ├── CMakeLists.txt
│   ├── include/client/
│   │   ├── GameClient.hpp
│   │   ├── NetworkClient.hpp
│   │   ├── ClientPredictor.hpp
│   │   │
│   │   └── rendering/           # SFML implementation (client-specific)
│   │       ├── SFMLRenderer.hpp
│   │       ├── SFMLTexture.hpp
│   │       └── SFMLSprite.hpp
│   │
│   └── src/
│       ├── main.cpp                 # Client entry point
│       ├── GameClient.cpp
│       ├── NetworkClient.cpp
│       ├── ClientPredictor.cpp
│       │
│       └── rendering/
│           ├── SFMLRenderer.cpp
│           ├── SFMLTexture.cpp
│           └── SFMLSprite.cpp
│
├── server/                          # 🖥️ Server Application
│   ├── CMakeLists.txt
│   ├── include/server/
│   │   ├── GameServer.hpp
│   │   ├── NetworkServer.hpp
│   │   └── ServerLogic.hpp
│   │
│   └── src/
│       ├── main.cpp                 # Server entry point
│       ├── GameServer.cpp
│       ├── NetworkServer.cpp
│       └── ServerLogic.cpp
│
├── tests/                           # 🧪 Unit Tests
│   ├── CMakeLists.txt
│   ├── engine/
│   │   ├── test_entity_manager.cpp
│   │   ├── test_registry.cpp
│   │   ├── test_packet.cpp
│   │   └── test_collision.cpp
│   ├── game/
│   └── integration/
│
├── assets/                          # 🎨 Game Assets
│   ├── textures/
│   │   ├── player.png
│   │   ├── enemy1.png
│   │   ├── bullet.png
│   │   └── starfield.png
│   ├── sounds/
│   │   ├── shoot.wav
│   │   └── explosion.wav
│   └── config/
│       ├── game_config.json
│       └── network_config.json
│
└── external/                        # 📦 Dependencies (managed by Conan/vcpkg)
    └── .gitignore
```

---

## 📝 Conventions de Nommage

### Fichiers
- **Headers** : PascalCase + `.hpp`
  - Exemple : `EntityManager.hpp`, `CollisionSystem.hpp`
  
- **Source** : Même nom que header + `.cpp`
  - Exemple : `EntityManager.cpp`

- **Tests** : `test_` + snake_case + `.cpp`
  - Exemple : `test_entity_manager.cpp`

### Classes
- **PascalCase** : `EntityManager`, `Registry`, `MovementSystem`

### Interfaces
- **Préfixe I** : `IRenderer`, `ISocket`, `ISystem`

### Namespaces
```cpp
namespace rtype {
    namespace engine {
        namespace ecs { /* ... */ }
        namespace network { /* ... */ }
    }
    namespace game { /* ... */ }
}

// Usage
using namespace rtype::engine::ecs;
```

### Includes Guards
```cpp
// Dans engine/include/engine/ecs/Registry.hpp
#ifndef RTYPE_ENGINE_ECS_REGISTRY_HPP
#define RTYPE_ENGINE_ECS_REGISTRY_HPP

// Code...

#endif // RTYPE_ENGINE_ECS_REGISTRY_HPP
```

Ou avec pragma once (plus simple) :
```cpp
#pragma once

// Code...
```

---

## 🔨 CMakeLists.txt Structure

### Root CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)
project(rtype VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_CLIENT "Build client" ON)
option(BUILD_SERVER "Build server" ON)

# Find dependencies
find_package(SFML COMPONENTS graphics window system REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

# Subdirectories
add_subdirectory(engine)
add_subdirectory(game)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_SERVER)
    add_subdirectory(server)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

### engine/CMakeLists.txt
```cmake
# Game Engine Library
add_library(engine STATIC
    # ECS
    src/ecs/EntityManager.cpp
    src/ecs/Registry.cpp
    src/ecs/SystemManager.cpp
    
    # Core
    src/core/Time.cpp
    src/core/Logger.cpp
    src/core/EventBus.cpp
    src/core/Config.cpp
    src/core/InputManager.cpp
    
    # Network
    src/network/Packet.cpp
    src/network/UDPSocket.cpp
    src/network/Connection.cpp
    src/network/ConnectionManager.cpp
    
    # Rendering (ABSTRACT ONLY - no SFML here!)
    src/rendering/Window.cpp
    src/rendering/Camera.cpp
    
    # Physics
    src/physics/CollisionDetector.cpp
    src/physics/QuadTree.cpp
    
    # Systems
    src/systems/MovementSystem.cpp
    src/systems/CollisionSystem.cpp
    src/systems/RenderSystem.cpp
    src/systems/AnimationSystem.cpp
)

target_include_directories(engine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(engine
    PUBLIC
        Boost::system
)

# Compiler warnings
if(MSVC)
    target_compile_options(engine PRIVATE /W4 /WX)
else()
    target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
```

### client/CMakeLists.txt
```cmake
add_executable(r-type_client
    src/main.cpp
    src/GameClient.cpp
    src/NetworkClient.cpp
    src/ClientPredictor.cpp
    
    # SFML implementation (client-specific)
    src/rendering/SFMLRenderer.cpp
    src/rendering/SFMLTexture.cpp
    src/rendering/SFMLSprite.cpp
)

target_include_directories(r-type_client 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r-type_client
    PRIVATE
        engine
        game
        sfml-graphics
        sfml-window
        sfml-system
)
```

### server/CMakeLists.txt
```cmake
add_executable(r-type_server
    src/main.cpp
    src/GameServer.cpp
    src/NetworkServer.cpp
    src/ServerLogic.cpp
)

target_include_directories(r-type_server 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r-type_server
    PRIVATE
        engine
        game
)

# Server doesn't need SFML graphics/window
target_compile_definitions(r-type_server PRIVATE NO_GRAPHICS)
```

---

## 🎯 Ordre de Création des Fichiers

### Phase 1 : ECS (Semaine 1)
```bash
engine/include/engine/ecs/
├── Entity.hpp              # 1. Type Entity
├── EntityManager.hpp       # 2. Gestion IDs
├── SparseSet.hpp          # 3. Storage
├── ComponentManager.hpp    # 4. Template wrapper
├── Registry.hpp           # 5. Hub central
├── System.hpp             # 6. Interface
├── SystemManager.hpp      # 7. Orchestration
└── View.hpp               # 8. Queries

engine/src/ecs/
├── EntityManager.cpp
├── Registry.cpp
└── SystemManager.cpp
```

### Phase 2 : Core (Semaine 2)
```bash
engine/include/engine/core/
├── Types.hpp              # Vector2, Rect, Color
├── Time.hpp
├── Logger.hpp
├── Event.hpp
├── EventBus.hpp
├── ResourceManager.hpp
├── Config.hpp
└── InputManager.hpp

engine/src/core/
├── Time.cpp
├── Logger.cpp
├── EventBus.cpp
├── Config.cpp
└── InputManager.cpp
```

### Phase 3 : Rendering (Semaine 3)
```bash
# ENGINE : Interfaces abstraites seulement
engine/include/engine/rendering/
├── IRenderer.hpp          # Interface abstraite
├── ITexture.hpp           # Interface abstraite
├── ISprite.hpp            # Interface abstraite
├── Window.hpp             # Wrapper générique
└── Camera.hpp             # Logique pure (pas de lib)

engine/src/rendering/
├── Window.cpp
└── Camera.cpp

# CLIENT : Implémentation SFML
client/include/client/rendering/
├── SFMLRenderer.hpp       # Implémente IRenderer
├── SFMLTexture.hpp        # Implémente ITexture
└── SFMLSprite.hpp         # Implémente ISprite

client/src/rendering/
├── SFMLRenderer.cpp
├── SFMLTexture.cpp
└── SFMLSprite.cpp

# SYSTEMS (dans engine, utilisent les interfaces)
engine/include/engine/systems/
└── RenderSystem.hpp

engine/src/systems/
└── RenderSystem.cpp
```

### Phase 4 : Network (Semaine 3)
```bash
engine/include/engine/network/
├── Packet.hpp
├── ISocket.hpp
├── UDPSocket.hpp
├── Connection.hpp
├── ConnectionManager.hpp
├── NetworkInterpolator.hpp
└── PacketTypes.hpp

engine/src/network/
├── Packet.cpp
├── UDPSocket.cpp
├── Connection.cpp
├── ConnectionManager.cpp
└── NetworkInterpolator.cpp
```

### Phase 5 : Physics & Systems (Semaine 4)
```bash
engine/include/engine/physics/
├── CollisionDetector.hpp
├── Collider.hpp
├── QuadTree.hpp
└── Ray.hpp

engine/src/physics/
├── CollisionDetector.cpp
└── QuadTree.cpp

engine/include/engine/systems/
├── MovementSystem.hpp
├── CollisionSystem.hpp
├── AnimationSystem.hpp
├── LifetimeSystem.hpp
└── ParticleSystem.hpp

engine/src/systems/
├── MovementSystem.cpp
├── CollisionSystem.cpp
├── AnimationSystem.cpp
├── LifetimeSystem.cpp
└── ParticleSystem.cpp
```

---

## 📦 Dépendances à Installer

### Avec Conan (recommandé)
```ini
# conanfile.txt
[requires]
sfml/2.6.1
boost/1.82.0

[generators]
CMakeDeps
CMakeToolchain

[options]
sfml:shared=False
sfml:graphics=True
sfml:window=True
sfml:audio=True
```

```bash
# Install
conan install . --output-folder=build --build=missing
cmake --preset conan-default
cmake --build build
```

### Avec vcpkg
```bash
vcpkg install sfml
vcpkg install boost-asio
vcpkg install boost-system

# Dans CMakeLists.txt
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
```

---

## 🧪 Structure des Tests

```
tests/
├── CMakeLists.txt
├── engine/
│   ├── ecs/
│   │   ├── test_entity_manager.cpp
│   │   ├── test_registry.cpp
│   │   └── test_view.cpp
│   ├── core/
│   │   ├── test_event_bus.cpp
│   │   └── test_resource_manager.cpp
│   ├── network/
│   │   ├── test_packet.cpp
│   │   └── test_connection.cpp
│   └── physics/
│       └── test_collision.cpp
└── main.cpp                # Test runner
```

### tests/CMakeLists.txt
```cmake
find_package(GTest REQUIRED)

add_executable(engine_tests
    main.cpp
    engine/ecs/test_entity_manager.cpp
    engine/ecs/test_registry.cpp
    engine/core/test_event_bus.cpp
    engine/network/test_packet.cpp
)

target_link_libraries(engine_tests
    PRIVATE
        engine
        GTest::GTest
        GTest::Main
)

add_test(NAME EngineTests COMMAND engine_tests)
```

---

## ✅ Checklist de Création

- [ ] Créer structure de base `engine/include/engine/`
- [ ] Créer sous-dossiers : `ecs/`, `core/`, `network/`, `rendering/`, `physics/`, `systems/`
- [ ] Créer `engine/src/` avec même structure
- [ ] Setup CMakeLists.txt (root + engine)
- [ ] Créer premier fichier : `engine/include/engine/ecs/Entity.hpp`
- [ ] Setup Conan/vcpkg pour dépendances
- [ ] Setup tests avec GTest
- [ ] Créer .gitignore

### .gitignore
```gitignore
# Build
build/
cmake-build-*/
*.o
*.a

# IDE
.vscode/
.idea/
*.swp

# Dependencies
external/
conan/

# Binaries
r-type_client
r-type_server
*.exe
```

---

**Structure prête ! Commence à coder ! 🚀**
