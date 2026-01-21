# 🚀 R-Type Multiplayer Shooter

Welcome to **R-Type**, a modern multiplayer shooter game built with a sophisticated Entity-Component-System architecture and real-time network synchronization. This project brings classic arcade shooter gameplay to a multiplayer experience with advanced engine capabilities.

---

## 📋 Table of Contents

- [About](#-about)
- [Architecture Overview](#️-architecture-overview)
- [Project Structure](#-project-structure)
- [Key Features](#-key-features)
- [Getting Started](#-getting-started)
- [Build Instructions](#-build-instructions)
- [Running the Game](#-running-the-game)
- [Architecture Documentation](#-architecture-documentation)
- [Development](#️-development)
- [License](#-license)
- [Credits](#-credits)

---

## 🎯 About

R-Type is a modern reimagining of the classic arcade shooter, featuring:

- 🔥 **Real-time multiplayer gameplay** with client-server architecture
- 🏗️ **Advanced Entity-Component-System (ECS)** for flexible game design
- 🎮 **Smooth rendering** with SFML for graphics and audio
- 🌐 **Network synchronization** with ASIO-based UDP communication
- 📜 **Lua scripting** for dynamic gameplay customization
- 🎯 **Gameplay mechanics** including power-ups, enemy waves, and boss battles

---

## 🏗️ Architecture Overview

```
┌────────────────────────────────────────┐
│     Game Module (game-specific)        │
│  • Gameplay mechanics                  │
│  • Player controls & Enemy AI          │
│  • Power-ups & Progression             │
└────────────────┬───────────────────────┘
                 │
                 ▼
┌────────────────────────────────────────┐
│    Engine Module (game-agnostic)       │
│  • ECS Core System                     │
│  • Rendering, Audio, Input             │
│  • Networking & Lua scripting          │
└────────────────┬───────────────────────┘
                 │
        ┌────────┴────────┐
        │                 │
   ┌────▼─────┐     ┌────▼──────┐
   │  Client  │     │  Server   │
   │  Module  │     │  Module   │
   │ • Input  │     │ • Auth    │
   │ • Render │     │ • Logic   │
   │ • Sync   │     │ • Broadcast│
   └──────────┘     └───────────┘
```

### Key Architectural Patterns

- **Entity-Component-System (ECS)**: Pure data-oriented design separating data (components) from logic (systems)
- **Client-Server Model**: Server as authoritative source of truth with clients sending input and receiving state updates
- **Modular Design**: Independent modules for engine, game, client, and server with clear interfaces

---

## 📁 Project Structure

The project consists of four main components:

| Component | Language | Description |
|-----------|----------|-------------|
| **Engine** (`engine/`) | C++17 | Core game engine with ECS, rendering, audio, and networking |
| **Game** (`game/`) | C++17 | Game-specific logic, enemy AI, power-ups, and progression |
| **Server** (`server/`) | C++17 | Authoritative game server with client management and state synchronization |
| **Client** (`client/`) | C++17 | Player client with input handling, rendering, and network communication |
| **Tests** (`tests/`) | C++17 | Unit and integration tests using GoogleTest |

---

## ⭐ Key Features

### 🎮 Gameplay Features

- **Multiplayer Shooting**: Real-time competitive/cooperative gameplay
- **Power-up System**: Collectible upgrades for weapons and abilities
- **Enemy Waves**: Progressive difficulty with varied enemy types
- **Boss Battles**: Challenging multi-phase boss encounters
- **Scrolling Background**: Dynamic parallax scrolling environments

### 🔧 Technical Features

- **ECS Architecture**: Flexible, data-oriented design for rapid iteration
- **Network Synchronization**: Smooth multiplayer with client prediction
- **Lua Scripting**: Dynamic game behavior customization
- **Asset Management**: Efficient loading of textures, sounds, and fonts
- **Input System**: Configurable controls with gamepad support
- **Audio System**: Spatial audio with SFML integration

---

## 🚀 Getting Started

### Prerequisites

- C++17 Compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.15+
- SFML 2.6+ (Graphics, Audio, Network)
- Lua 5.3+ (Scripting)
- ASIO 1.18+ (Networking)
- GoogleTest (Testing, optional)

### System Dependencies

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    libsfml-dev \
    liblua5.3-dev \
    libasio-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    build-essential \
    cmake \
    git
```

#### macOS

```bash
brew install sfml lua asio cmake
```

#### Windows

1. Install [vcpkg](https://github.com/microsoft/vcpkg)
2. Install dependencies:
   ```bash
   vcpkg install sfml lua asio
   ```

---

## 🔨 Build Instructions

### Clone and Build

```bash
# Clone the repository
git clone <repository-url>
cd R-type

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CLIENT=ON \
    -DBUILD_SERVER=ON \
    -DBUILD_GAME=ON \
    -DBUILD_TESTS=OFF

# Build the project
make -j$(nproc)
```

### Build Options

| CMake Option | Default | Description |
|--------------|---------|-------------|
| `BUILD_CLIENT` | ON | Build the client executable |
| `BUILD_SERVER` | ON | Build the server executable |
| `BUILD_GAME` | ON | Build the game module |
| `BUILD_TESTS` | OFF | Build unit tests |
| `CMAKE_BUILD_TYPE` | Release | Debug/Release/RelWithDebInfo |

### Separate Component Builds

```bash
# Build only server
cmake .. -DBUILD_CLIENT=OFF -DBUILD_GAME=OFF -DBUILD_SERVER=ON
make RType_server

# Build only client
cmake .. -DBUILD_SERVER=OFF -DBUILD_GAME=OFF -DBUILD_CLIENT=ON
make RType_client

# Build only game (for testing)
cmake .. -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF -DBUILD_GAME=ON
make RType_game
```

---

## 🎮 Running the Game

### 1. Start the Game Server

```bash
./build/server/RType_server [options]
```

**Server Options:**

| Option | Description | Default |
|--------|-------------|---------|
| `--port` | Port to listen on | 4242 |
| `--max-players` | Maximum connected players | 4 |
| `--tick-rate` | Server tick rate (Hz) | 60 |
| `--map` | Game map to load | default |
| `--verbose` | Enable verbose logging | false |

**Example:**

```bash
./build/server/RType_server --port 4242 --max-players 4 --tick-rate 60
```

### 2. Start the Game Client

```bash
./build/client/RType_client [options]
```

**Client Options:**

| Option | Description | Default |
|--------|-------------|---------|
| `--server` | Server IP address | localhost |
| `--port` | Server port | 4242 |
| `--player-name` | Player display name | Player |
| `--fullscreen` | Start in fullscreen mode | false |
| `--resolution` | Window resolution | 1920x1080 |

**Example:**

```bash
./build/client/RType_client --server 127.0.0.1 --port 4242 --player-name "Player1"
```

### 3. Direct Game Execution (Single Player)

```bash
./build/game/RType_game [options]
```

**Options:**

| Option | Description | Default |
|--------|-------------|---------|
| `--level` | Starting level | 1 |
| `--difficulty` | Game difficulty (easy/medium/hard) | medium |
| `--fullscreen` | Fullscreen mode | false |

---

## 📚 Architecture Documentation

### Core Engine Architecture

| Document | Purpose | Key Topics |
|----------|---------|------------|
| **Engine Module - Detailed** | Complete engine architecture | Rendering, Audio, ECS, Scripting, Networking |
| **ECS System - Deep Dive** | Entity-Component-System pattern | Components, Entities, Systems, Coordinator |

### Multiplayer Architecture

| Document | Purpose | Key Topics |
|----------|---------|------------|
| **Client Module** | Client implementation | Connection management, Input, State sync |
| **Server Module** | Server implementation | Client management, Authoritative state, Broadcasting |

### Game-Specific Architecture

| Document | Purpose | Key Topics |
|----------|---------|------------|
| **Game Module** | Gameplay mechanics | Player controls, Enemy AI, Power-ups, Stages |

### Quick Navigation Guide

- **New to the project?** Start with the Architecture Overview
- **Working on engine systems?** Read Engine Detailed
- **Implementing game features?** Read ECS System and Game Module
- **Working on networking?** Read Client Module and Server Module

---

## 🛠️ Development

### Running Tests

```bash
# Build tests
cd build
cmake .. -DBUILD_TESTS=ON
make unit_tests

# Run tests
./tests/unit_tests
```

### Code Structure

```
R-type/
├── engine/              # Core game engine
│   ├── include/         # Public headers
│   ├── src/             # Source files
│   ├── modules/         # Optional engine modules
│   └── CMakeLists.txt
├── game/                # Game-specific logic
│   ├── include/
│   ├── src/
│   └── CMakeLists.txt
├── client/              # Client application
│   ├── include/
│   ├── src/
│   └── CMakeLists.txt
├── server/              # Server application
│   ├── include/
│   ├── src/
│   └── CMakeLists.txt
├── tests/               # Unit tests
│   ├── CMakeLists.txt
│   └── NetworkTests.cpp
├── assets/              # Game assets
├── documentation/       # Documentation
├── cmake/               # CMake utilities
└── CMakeLists.txt       # Root CMake
```

### Key Systems

**ECS Components:**
- Position, Velocity, Sprite, Health, Input, Weapon
- NetworkId, Player, Enemy, Projectile, PowerUp

**Engine Systems:**
- **RenderSystem**: Handles drawing of entities
- **MovementSystem**: Updates positions based on velocities
- **InputSystem**: Processes player input
- **NetworkSystem**: Manages network communication
- **AudioSystem**: Plays sound effects and music
- **AnimationSystem**: Handles sprite animations

### Contribution Guidelines

- **Code Style**: Follow existing code conventions
- **Testing**: Add tests for new features
- **Documentation**: Update relevant documentation
- **Architecture**: Maintain separation of concerns
- **Performance**: Optimize for 60 FPS gameplay

---

## 🔧 Troubleshooting

### Common Issues

- **Missing dependencies**: Ensure all system packages are installed
- **SFML linking errors**: Check SFML installation paths
- **Network connection issues**: Verify server is running and ports are open
- **Build failures**: Clean build directory and reconfigure CMake

### Debug Mode

```bash
# Build in debug mode
cmake .. -DCMAKE_BUILD_TYPE=Debug
make clean && make

# Run with debug output
./build/client/RType_client --verbose
```

---

## 📄 License

This project is developed as part of an educational curriculum. See the repository for specific license information.

---

## 👥 Credits

Developed with modern C++ practices, leveraging:

- **SFML** for multimedia
- **ASIO** for networking
- **Lua** for scripting
- **GoogleTest** for testing
- **CMake** for build system

For detailed API documentation and internal architecture, see the `documentation/` directory.
