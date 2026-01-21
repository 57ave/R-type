# R-Type Build System & Dependencies

This document explains the "Self-Contained" build system of the R-Type project, how it manages dependencies (Lua, Sol3, SFML), and how to generate a distributable version of the game.

## 1. "Self-Contained" Philosophy

**Why?**
The goal is to allow any developer (or CI/CD) to clone the repo and compile the project **without manually installing complex libraries** on their system (like Lua 5.4, Sol3, or even SFML). This ensures:
- **Reproducibility**: Everyone uses exactly the same library versions.
- **Simplicity**: No "DLL hell" or system installation conflicts.
- **Portability**: Works on Linux, Windows, and macOS with minimal prerequisites (CMake, C++ Compiler).

**How?**
We use **CPM (CMake Package Manager)**. It's a small CMake script that:
1.  Checks if a dependency is already available on the system (via `find_package`).
2.  If absent, **it automatically downloads it** from GitHub (source code).
3.  It integrates it directly into the project build (as if it were your own code).

## 2. Dependency Management

### Lua 5.4 & Sol3
The `engine/CMakeLists.txt` file handles this:
- **Check**: It first attempts to find Lua on the system.
- **CPM Fallback**: If Lua is not found, CPM downloads `epezent/lua` (a "CMake-friendly" version of Lua 5.4) and compiles it as a static library.
- **Sol3**: Same for Sol3 (C++/Lua binding), which is downloaded from GitHub and added as a "Header Only" library.

### SFML 2.6
SFML is also managed by CPM if not found, guaranteeing that the graphics engine is always available.

## 3. Installation & Packaging

The build doesn't just compile executables. It defines **installation** targets (`install targets`) to create a clean folder containing everything needed to play.

The `install(TARGETS ...)` and `install(DIRECTORY ...)` commands in `CMakeLists.txt` tell CMake:
- Where to put executables (`bin/`)
- Where to put libraries (`lib/`)
- Where to put assets (`bin/assets`)

## 4. Usage Guide

### Prerequisites
- CMake 3.15+
- C++ Compiler (GCC, Clang, MSVC)
- *Optional*: Ninja (for faster builds)

### Compile and Install

The standard procedure to get a playable game is as follows:

```bash
# 1. Configuration (Generate Makefiles)
# -B build : Creates build files in 'build' directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compilation
# --build build : Triggers compilation in 'build' directory
# -j : Uses all CPU cores
cmake --build build -j

# 3. Installation
# --install build : Copies final files to default install directory
# (On Linux, default is /usr/local, but you can specify a local folder with --prefix)
cmake --install build --prefix ./install
```

### Result

After step 3, you will have an `install/` folder structured like this:

```text
install/
├── bin/
│   ├── r-type_game       # Game Client
│   ├── r-type_server     # Dedicated Server
│   └── assets/           # Sprites, sounds, scripts (copied automatically)
└── lib/                  # Static/Dynamic Libraries
```

This `install` folder is **autonomous**. You can zip it and send it to another user (on the same OS), and they can play immediately.

### For Developers (Quick Test)
You can still run executables from the `build` folder, but remember that for asset loading, the game typically expects to find the `assets/` folder next to the executable or in the current working directory. The installation step solves this issue permanently.
