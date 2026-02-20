# R-Type

A multiplayer arcade shooter built in C++17, using a custom ECS engine, UDP networking, and Lua scripting for game configuration.

The project is split into four modules: **engine**, **game**, **server**, and **client**. The server runs all game logic authoritatively; clients handle input and rendering only.

---

## Requirements

- CMake 3.15+
- C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- Dependencies (SFML, Lua, Sol3, ASIO) are fetched automatically via CPM at build time

**Linux/macOS additional packages:**

```bash
# Ubuntu/Debian
sudo apt-get install -y libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
    libopenal-dev libflac-dev libvorbis-dev libgl1-mesa-dev build-essential cmake git
```

---

## Build

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix ./install
```

Binaries are placed in `install/bin/`.

### Windows (native)

```powershell
.\build_windows.bat
```

### Windows (cross-compile from Linux)

```bash
chmod +x scripts/build_windows.sh
./scripts/build_windows.sh
```

Output is placed in `release_windows/bin/`.

---

## Run

Start the server first, then launch the client:

```bash
# Linux/macOS
./install/bin/r-type_server
./install/bin/r-type_game

# Windows
.\release_windows\bin\r-type_server.exe
.\release_windows\bin\r-type_game.exe
```

The server listens on UDP port **4242** by default.

---

## Project Structure

```
engine/     Core ECS, rendering (SFML), audio, networking, Lua scripting
game/       R-Type game logic, states, enemy AI, Lua configs
server/     Authoritative game server, room management, state broadcasting
client/     Thin client: input capture, rendering, network sync
docs/       Technical documentation
```

---

## Documentation

| File | Description |
|---|---|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Engine modules, ECS design, data flow |
| [docs/PROTOCOL.md](docs/PROTOCOL.md) | UDP packet format and protocol reference |
| [docs/LUA_SCRIPTING.md](docs/LUA_SCRIPTING.md) | Lua configuration system and API |
| [docs/BUILD.md](docs/BUILD.md) | Detailed build instructions for all platforms |
| [docs/RELEASE.md](docs/RELEASE.md) | Release workflow via GitHub Actions |
| [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md) | Contribution guidelines |

---

## License

Educational project. See team credits in commit history.
