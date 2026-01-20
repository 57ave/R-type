# R-Type Windows Cross-Compilation Guide

This document explains how to compile and run the R-Type game for Windows from a Linux environment using MinGW-w64.

## Prerequisites

You need the following packages installed on your Linux system (Debian/Ubuntu):

```bash
sudo apt update
sudo apt install g++-mingw-w64-x86-64 cmake git python3
```

## How to Build

We provide a script to automate the entire process (fetching dependencies, patching, and compiling):

```bash
# From the project root
chmod +x scripts/build_windows.sh
./scripts/build_windows.sh
```

The script will:
1.  Use the `cmake/Toolchain-MinGW.cmake` toolchain.
2.  Fetch and build **Lua 5.4.6** from the official source.
3.  Fetch **OpenAL-Soft** (required for Windows audio).
4.  Fetch and patch **SFML 2.6.1** (to resolve conflicts between SFML's internal OpenAL and our CPM version).
5.  Compile all engine modules, the game client, and the server.

### Build Outputs

The Windows distribution is automatically packaged in:
- `release_windows/bin/`

This folder contains:
- `r-type_game.exe`
- `r-type_server.exe`
- All required `.dll` files (SFML, OpenAL, Lua, and engine modules)
- `assets/` directory

## How to Launch (on Windows)

To run the game on a Windows machine:

1.  **Transfer the Package**:
    - Zip the **entire** content of the `release_windows/bin/` directory.
    - Extract it on your Windows machine.
2.  **Run the Server**:
    - Launch `r-type_server.exe`
3.  **Run the Game**:
    - Launch `r-type_game.exe`

## Technical Details (Implementation)

Several cross-platform adaptations were made to support Windows:

- **Network**: Explicitly linked `ws2_32` and `wsock32` libraries.
- **Dynamic Loading**: Refactored `SystemLoader.cpp` to use `LoadLibraryA` and `GetProcAddress` on Windows.
- **Endianness**: Fixed missing `<endian.h>` in `Packet.hpp`.
- **Macro Conflicts**: Resolved conflicts between `windows.h` macros (`ERROR`, `INFO`) and the internal `DevConsole` enum.
- **Packaging**: Simplified distribution by using `cmake --install` to gather all DLLs and assets in one place.

## Troubleshooting

- **Missing DLLs**: Ensure you have copied the **entire** content of `release_windows/bin/`. All dependencies are gathered there by the build script.
- **Assets**: If textures or sounds are missing, check that the `assets/` folder is present in the same directory as the `.exe`.

