# R-Type Project

This project constitutes a network-based R-Type game, featuring a split client-server architecture.

## 📋 Requirements

To build and run this project, you need:
- **Windows 10/11**
- **CMake** (3.14 or newer)
- **Visual Studio 2022** (Must include the **"Desktop development with C++"** workload during installation)
- **Git**

## 🛠️ Build Instructions

We have verified that the build process is automated. You do not need to manually install libraries like SFML or Boost; the script handles it for you using `vcpkg`.

### Automatic Build (Recommended)

1.  Open a terminal (PowerShell or Command Prompt).
2.  Navigate to the project root.
3.  Run the build script:
    ```powershell
    .\build_windows.bat
    ```

This script will:
- Check for `vcpkg`.
- Download and bootstrap `vcpkg` if it's missing.
- Install necessary dependencies (SFML, Boost).
- Configure the project with CMake.
- Compile both the Client and Server.

The executables will be generated in:
- `build/windows-release/r-type_client.exe`
- `build/windows-release/r-type_server.exe`

### Manual Build

If you prefer to run commands manually:

1.  **Setup Vcpkg**:
    ```powershell
    git clone https://github.com/microsoft/vcpkg.git
    .\vcpkg\bootstrap-vcpkg.bat
    ```

2.  **Configure**:
    ```powershell
    cmake --preset windows-release
    ```

3.  **Build**:
    ```powershell
    cmake --build --preset windows-release
    ```

## 🚀 Running the Game

### Start the Server
Open a terminal and run:
```powershell
.\build\windows-release\r-type_server.exe
```

### Start the Client
Open another terminal and run:
```powershell
.\build\windows-release\r-type_client.exe
```

## 📂 Project Structure

- `src/client/`: Code specific to the game client (graphics, input).
- `src/server/`: Code specific to the game server (game state, logic).
- `vcpkg.json`: Manifest file listing dependencies (SFML, Boost).
- `CMakeLists.txt`: Build configuration.
