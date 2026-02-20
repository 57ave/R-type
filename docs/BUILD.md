# Build System

## How Dependencies Are Managed

All external libraries (SFML 2.6, Lua 5.4, Sol3, ASIO) are handled by **CPM (CMake Package Manager)**. CPM is a small CMake script included in the project that:

1. Checks if the library is already installed on the system.
2. If not, downloads the source from GitHub and compiles it as part of the project build.

You do not need to install any of these libraries manually. The only hard requirements are CMake and a C++17 compiler.

---

## Linux / macOS

### System packages (Ubuntu/Debian)

```bash
sudo apt-get install -y \
    libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
    libopenal-dev libflac-dev libvorbis-dev \
    libgl1-mesa-dev libglu1-mesa-dev \
    build-essential cmake git
```

### Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compile (uses all available CPU cores)
cmake --build build -j

# Install into ./install/
cmake --install build --prefix ./install
```

After install, the `install/` directory is self-contained:

```
install/
├── bin/
│   ├── r-type_game      # game client
│   ├── r-type_server    # dedicated server
│   └── assets/          # sprites, sounds, scripts
└── lib/                 # shared libraries
```

You can zip `install/` and distribute it to another machine running the same OS.

### Quick run (without install)

```bash
./build/game/r-type_game
./build/server/r-type_server
```

Note: assets must be reachable from the working directory. The install step handles this automatically.

---

## Windows (Native)

Requires Visual Studio 2019+ with the C++ workload, or MinGW-w64.

```powershell
.\build_windows.bat
```

The script configures CMake and builds both the client and server. Output goes to `build/`.

---

## Windows (Cross-compile from Linux)

Requires MinGW-w64 on your Linux machine:

```bash
sudo apt install g++-mingw-w64-x86-64 cmake git python3
```

Then run:

```bash
chmod +x scripts/build_windows.sh
./scripts/build_windows.sh
```

The script:
1. Uses `cmake/Toolchain-MinGW.cmake` as the cross-compilation toolchain.
2. Fetches and builds Lua 5.4.6 from source.
3. Fetches OpenAL-Soft (required for audio on Windows).
4. Fetches and patches SFML 2.6.1 (resolves OpenAL conflicts with the CPM version).
5. Compiles all modules and packages the result.

Output is placed in `release_windows/bin/`:

```
release_windows/bin/
├── r-type_game.exe
├── r-type_server.exe
├── *.dll                # SFML, OpenAL, Lua, engine modules
└── assets/
```

To run on a Windows machine, transfer the entire `release_windows/bin/` directory (do not separate the DLLs from the executables).

---

## CMake Options

| Option | Default | Description |
|---|---|---|
| `BUILD_CLIENT` | ON | Build the client executable |
| `BUILD_SERVER` | ON | Build the server executable |
| `BUILD_GAME` | ON | Build the game module |
| `BUILD_TESTS` | OFF | Build unit tests |
| `CMAKE_BUILD_TYPE` | Release | `Release` or `Debug` |

Example with options:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build -j
```

---

## Windows-Specific Notes

Several adaptations were made to support Windows:

- **Networking**: explicitly links `ws2_32` and `wsock32`.
- **Dynamic loading**: `SystemLoader.cpp` uses `LoadLibraryA` / `GetProcAddress` on Windows instead of `dlopen`.
- **Endianness**: `<endian.h>` is not available on Windows; replaced with a portable alternative in `Packet.hpp`.
- **Macro conflicts**: `windows.h` defines `ERROR` and `INFO` macros that clash with the internal `DevConsole` enum — resolved with `#undef`.
