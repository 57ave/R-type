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
   │ • Broadcast│     │ • State   │
   └──────────┘     └───────────┘
```

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
- SFML 2.6+ (Managed automatically via CPM)
- Lua 5.3+ (Optional, managed via CPM)

### System Dependencies (Linux/macOS)

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
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
brew install cmake
```

---

## 🔨 Build Instructions

### Windows (Automated)

We provide a batch script to automate the build process on Windows.

1.  Open a terminal (PowerShell or Command Prompt).
2.  Run the build script:
    ```powershell
    .\build_windows.bat
    ```
    This script will check for CMake, configure the project (downloading dependencies via CPM), and build both Client and Server.

### Manual Build (All Platforms)

```bash
# Clone the repository
git clone <repository-url>
cd R-type

# Create build directory
mkdir build && cd build

# Configure with CMake (Download dependencies)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release
```

### Build Options

| CMake Option | Default | Description |
|--------------|---------|-------------|
| `BUILD_CLIENT` | ON | Build the client executable |
| `BUILD_SERVER` | ON | Build the server executable |
| `BUILD_GAME` | ON | Build the game module |
| `BUILD_TESTS` | OFF | Build unit tests |

---

## 🎮 Running the Game

### Start the Server
```bash
# Windows
.\build\server\Release\RType_server.exe
# Linux/Mac
./build/server/RType_server
```

### Start the Client
```bash
# Windows
.\build\client\Release\RType_client.exe
# Linux/Mac
./build/client/RType_client
```

---

## 📄 License

This project is developed as part of an educational curriculum.

---

## 👥 Credits

Developed with modern C++ practices, leveraging:
- **SFML** for multimedia
- **ASIO** for networking
- **Lua** for scripting
- **GoogleTest** for testing
- **CMake** for build system

