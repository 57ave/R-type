# R-Type: Windows Cross-Compilation & Usage Guide

This document outlines the procedure for cross-compiling the R-Type project from Linux to Windows and provides instructions for running the distributed binaries.

## 1. Cross-Compilation (Linux to Windows)

We utilize the **MinGW-w64** toolchain to generate native Windows executables (`.exe`) from a Linux development environment.

### Prerequisites (Debian/Ubuntu)
Ensure the following tools are installed:
```bash
sudo apt install mingw-w64 cmake
```

### Build & Package
To generate a standalone release, execute the following command from the project root:

```bash
./cross_compile_windows.sh && ./package_windows.sh
```

This process will:
1.  Configure the build using the `Toolchain-MinGW-w64.cmake` file.
2.  Compile all binaries for Windows x64.
3.  Generate a `release-windows/` directory containing all necessary executables, DLLs (SFML, MinGW runtime), and assets.

---

## 2. Architecture Overview

The release package contains three main executables:

*   **`r-type_server.exe`** (Host)
    *   **Role**: The authoritative ECS game server.
    *   **Port**: Listens on `12345` by default.

*   **`r-type_game.exe`** (Network Client)
    *   **Role**: The primary game client that connects to the server.
    *   **Features**: Full ECS integration, network synchronization, and asset handling.

*   **`r-type_client.exe`** (Prototype - Deprecated)
    *   **Role**: A standalone graphical prototype used for initial asset testing. It does not support networking.

---

## 3. How to Play (Network Mode)

To start a multiplayer session, follow these steps using **Windows PowerShell** or **Command Prompt** within the `release-windows/` directory.

### Step 1: Start the Server
Open a terminal and launch the server first:
```powershell
./r-type_server.exe
```
*Wait for the confirmation message: "GameServer Started on port 12345".*

### Step 2: Start the Client
Open a **second** terminal and launch the game client:
```powershell
./r-type_game.exe --network 127.0.0.1 12345
```

> [!NOTE]
> The window will open immediately with a "Connecting..." status. Once the server accepts the connection, the game usually begins.

### Troubleshooting
*   **Visual Artifacts**: If assets fail to load, the game uses colored rectangles (Red/Green/Cyan) as placeholders to ensure gameplay continuity.
*   **Connection Issues**: Ensure you are using port **12345**, matching the server's default configuration.
