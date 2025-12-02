<div align="center">
  
# R-Type

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org/)

**A modern C++ recreation of the classic R-Type shooter game**

[Features](#-features) • [Getting Started](#-getting-started) • [Documentation](#-documentation) • [Contributing](#-contributing)

</div>

---

## 📋 Table of Contents

- [About](#-about)
- [Features](#-features)
- [Architecture](#️-architecture)
- [Getting Started](#-getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [Building](#building)
  - [Running](#running)
- [Usage](#-usage)
- [Configuration](#️-configuration)
- [Documentation](#-documentation)
- [Testing](#-testing)
- [Deployment](#-deployment)
- [Contributing](#-contributing)
- [Contributors](#-contributors)
- [License](#-license)
- [Acknowledgments](#-acknowledgments)
- [Contact](#-contact)

---

## 🎮 About

R-Type is a networked multiplayer game built with modern C++ practices. This project aims to recreate the classic arcade experience while implementing a robust client-server architecture powered by our own custom Entity Component System (ECS).

Our implementation features a custom-built ECS architecture designed from scratch, providing efficient entity management, flexible component composition, and optimized system processing for game logic.

### Key Objectives

- Implement a scalable networked game architecture
- Design and develop a custom Entity Component System (ECS) from scratch
- Apply modern C++20 features and best practices
- Create a maintainable and extensible codebase
- Achieve high performance through data-oriented design

---

## ✨ Features

### Current Features

- blablabla

### Planned Features

- Client-server multiplayer functionality with ECS synchronization
- Advanced game entity management and component systems
- Physics engine integrated with ECS
- Audio and graphics rendering systems
- Collision detection and response
- AI systems for enemies and game logic

---

## 🏗️ Architecture

This project is built around a custom **Entity Component System (ECS)** architecture designed specifically for high-performance game development.

### ECS Architecture

The ECS pattern separates:
- **Entities**: Unique identifiers for game objects (players, enemies, projectiles, etc.)
- **Components**: Pure data structures (Position, Velocity, Health, Sprite, etc.)
- **Systems**: Logic that processes entities with specific component combinations

This architecture provides:
- **Performance**: Cache-friendly data layout and efficient iteration
- **Flexibility**: Easy creation of new entity types by combining components
- **Maintainability**: Clear separation of data and logic
- **Scalability**: Efficient handling of thousands of entities

```
R-Type/
├── assets/           # Game assets (sprites, sounds, etc.)
├── includes/         # Header files
│   ├── ecs/         # ECS core (Entity, Component, System classes)
│   ├── components/  # Component definitions
│   └── systems/     # System implementations
├── src/             # Source files
│   └── main.cpp     # Application entry point
├── build/           # Build output directory
├── CMakeLists.txt   # CMake configuration
└── README.md        # This file
```

### Technology Stack

- **Language**: C++20
- **Build System**: CMake 3.16+
- **[À COMPLÉTER]**: Add your libraries and frameworks
  - Graphics: SFML
  - Networking: [Boost.Asio / etc.]
  - Testing: [Google Test / Catch2 / etc.]

### Design Patterns

Our implementation leverages several key design patterns:
- **Entity Component System (ECS)**: Core architecture for game object management
- **Data-Oriented Design**: Optimized memory layout for performance
- **Observer Pattern**: Event handling and system communication
- **Factory Pattern**: Entity creation and component instantiation
- **Command Pattern**: Input handling and network commands

---

## 🚀 Getting Started

### Prerequisites

Before you begin, ensure you have the following installed:

- **C++ Compiler** with C++20 support:
  - GCC 10+ or Clang 11+ (Linux)
  - MSVC 19.29+ (Windows)
  - AppleClang 13+ (macOS)
- **CMake** 3.16 or higher
- **Git** for version control
- **[À COMPLÉTER]**: Add additional dependencies
  - SFML 2.5+
  - Boost 1.75+
  - etc.

### Installation

1. **Clone the repository**

```bash
git clone https://github.com/EpitechPGE3-2025/G-CPP-500-PAR-5-2-rtype-13.git
cd R-Type
```

2. **Install dependencies**

**[À COMPLÉTER]**: Add dependency installation instructions

<details>
<summary>Ubuntu/Debian</summary>

```bash
sudo apt update
sudo apt install build-essential cmake git
# Add your specific dependencies
# sudo apt install libsfml-dev libboost-all-dev
```
</details>

<details>
<summary>macOS</summary>

```bash
brew install cmake
# Add your specific dependencies
# brew install sfml boost
```
</details>

<details>
<summary>Windows</summary>

```bash
# Install Visual Studio 2019 or later with C++ tools
# Install CMake from https://cmake.org/download/
# Add instructions for your dependencies using vcpkg or manual installation
```
</details>

### Building

#### Quick Build

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# Or use make directly
make
```

#### Build Options

**[À COMPLÉTER]**: Add any CMake options you support

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..

# With tests
# cmake -DBUILD_TESTS=ON ..

# Custom installation prefix
# cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
```

For detailed CMake configuration instructions, see [CMAKE_GUIDE.md](CMAKE_GUIDE.md).

### Running

After building, run the executable:

```bash
# From the build directory
./R-Type

# Or from the project root
./build/R-Type
```

**[À COMPLÉTER]**: Add command-line arguments and options

```bash
# Example with arguments (to be implemented)
# ./R-Type --server --port 8080
# ./R-Type --client --host localhost --port 8080
```

---

## 📖 Usage

**[À COMPLÉTER]**: Add usage instructions

### Game Controls

- **[À COMPLÉTER]**: Document game controls
- Arrow Keys / WASD: Move
- Space: Shoot
- Esc: Pause/Menu
- etc.

### Server Setup

**[À COMPLÉTER]**: If applicable

```bash
# Start server
./R-Type --server --port 8080
```

### Client Connection

**[À COMPLÉTER]**: If applicable

```bash
# Connect to server
./R-Type --client --host localhost --port 8080
```

---

## ⚙️ Configuration

**[À COMPLÉTER]**: Document configuration files and options

Configuration files (if any) can be found in:
- `config/game.conf` - Game settings
- `config/server.conf` - Server configuration
- etc.

### Example Configuration

```ini
# config/game.conf
[game]
resolution = 1920x1080
fullscreen = false
fps_limit = 60

[audio]
master_volume = 100
music_volume = 80
sfx_volume = 100
```

---

## 📚 Documentation

Additional documentation can be found in the following files:

- [CMAKE_GUIDE.md](CMAKE_GUIDE.md) - CMake configuration guide
- **[À COMPLÉTER]**: Add links to other documentation
  - [ARCHITECTURE.md](docs/ARCHITECTURE.md) - Detailed architecture
  - [API.md](docs/API.md) - API documentation
  - [PROTOCOL.md](docs/PROTOCOL.md) - Network protocol specification
  - etc.

### Generating Documentation

**[À COMPLÉTER]**: If you use Doxygen or similar

```bash
# Generate Doxygen documentation
# doxygen Doxyfile
# Documentation will be in docs/html/index.html
```

---

## 🧪 Testing

**[À COMPLÉTER]**: Add testing instructions

```bash
# Build with tests
# cmake -DBUILD_TESTS=ON ..
# make

# Run tests
# ctest
# or
# ./build/tests/R-Type_tests
```

### Test Coverage

**[À COMPLÉTER]**: Add coverage instructions if applicable

```bash
# Generate coverage report
# cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
# make
# make coverage
```

---

## 🚢 Deployment

**[À COMPLÉTER]**: Add deployment instructions

### Creating a Release Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cmake --install . --prefix /opt/rtype
```

### Packaging

**[À COMPLÉTER]**: Add packaging instructions

```bash
# Create installer/package
# cpack
```

---

## 🤝 Contributing

We welcome contributions! Please follow these guidelines:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add some amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Coding Standards

- Follow C++20 best practices
- Use meaningful variable and function names
- Comment complex logic
- Write unit tests for new features
- Run clang-format before committing (if configured)

**[À COMPLÉTER]**: Add specific coding guidelines

### Commit Message Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
type: subject
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

---

## 👥 Contributors

|Picture| Name | Role | GitHub | Email |
|------|------|------|--------|-------|
|<img src="https://github.com/noelongueve.png" width="200em"/>| **Noé Longuève** | Game Design Developer | [@noelongueve](https://github.com/noelongueve) | noe.longueve@epitech.eu |
|<img src="https://github.com/ZEROUALWassim.png" width="200em"/>| **Wassim Zeroual** | ? Developer | [@ZEROUALWassim](https://github.com/ZEROUALWassim) | wassim.zeroual@epitech.eu |
|<img src="https://github.com/57ave.png" width="200em"/>| **Gustave Delecroix** | ECS Developer | [@57ave](https://github.com/57ave) | gustave.delecroix@epitech.eu |
|<img src="https://github.com/D3adPlays.png" width="200em"/>| **Matthieu Witrowiez** | Network Developer | [@D3adPlays](https://github.com/D3adPlays) | matthieu-florent-erwin.witrowiez@epitech.eu |
|<img src="https://github.com/AxiomePro.png" width="200em"/>| **Paul Colagrande** | Network Developer | [@AxiomePro](https://github.com/AxiomePro) | paul.colagrande@epitech.eu |

---

## 📄 License

**[À COMPLÉTER]**: Specify your license

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Or choose another license:
- Apache License 2.0
- GPL v3.0
- etc.

---

## 🙏 Acknowledgments

**[À COMPLÉTER]**: Add acknowledgments

- Original R-Type game by Irem (1987)
- [Library/Framework] for [specific feature]
- Inspiration from [other projects]
- [Course/Institution] for educational support
- etc.

---

<div align="center">

**Made with ❤️ by the R-Type Team**

[Back to top](#r-type)

</div>
