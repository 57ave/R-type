# CI/CD Pipeline Documentation

This document details the Continuous Integration and Continuous Deployment (CI/CD) pipeline for Granular. The pipeline is built using **GitHub Actions**.

## 🚀 Overview

The CI pipeline is triggered on:
- `push` events to any branch
- `pull_request` events targeting any branch

Its primary goal is to ensure:
1.  Code compilation on supported platforms settings (Linux GCC/Clang).
2.  Code style consistency via `clang-format`.
3.  Execution of the test suite.

## 🛠️ Active Workflows

The workflow is defined in `.github/workflows/build_test.yml`.

### 1. Linux (GCC)
- **Runs on**: `ubuntu-latest`
- **Compiler**: GCC `g++`
- **Steps**:
    - Installs system dependencies (SDL2 alternatives, config tools, etc.).
    - Configures CMake with `-DCMAKE_CXX_COMPILER=g++`.
    - Builds the project in `Release` mode.
    - Runs tests via `ctest`.

### 2. Linux (Clang)
- **Runs on**: `ubuntu-latest`
- **Compiler**: Clang `clang++`
- **Steps**:
    - Installs system dependencies and `clang`.
    - Configures CMake with `-DCMAKE_CXX_COMPILER=clang++`.
    - Builds and tests similar to the GCC job.

### 3. Code Formatting
- **Runs on**: `ubuntu-latest`
- **Tool**: `clang-format`
- **Steps**:
    - Checks if code adheres to the Google-based style guide defined in `.clang-format`.
    - Runs `./scripts/format.sh --check`.
    - **Failure**: If this job fails, run `./scripts/format.sh` locally to fix issues.

---

## ⚠️ Windows MSVC Build (Disabled)

Support for Windows CI (MSVC) was implemented but has been **temporarily removed** from the active pipeline to focus on core stability. However, the build system (`CMakeLists.txt`) retains full support for Windows.

### Configuration Details

If you wish to re-enable or reproduce the Windows build locally/in CI, here is the configuration:

- **Runner**: `windows-latest`
- **Compiler**: MSVC (Visual Studio 2022)
- **Dependency Manager**: `vcpkg`

#### How it works (Architecture)
1.  **Vcpkg Integration**:
    - The build uses `vcpkg` to manage dependencies (SFML, Lua, OpenAL-Soft).
    - CMake is configured with the toolchain file: `-DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"`.

2.  **SFML Detection**:
    - The root `CMakeLists.txt` uses `find_package(SFML ... CONFIG)` to locate the `vcpkg`-installed binaries.
    - **Important**: It avoids building SFML from source (CPM) on Windows to prevent linker errors (`optimized.lib` conflicts) and reduce build time.

#### Re-enabling in CI
To re-add the job, append the following to `.github/workflows/build_test.yml`:

```yaml
  windows-msvc:
    name: Windows (MSVC)
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Install Dependencies via vcpkg
        run: |
          vcpkg install sfml:x64-windows
          vcpkg install lua:x64-windows
          vcpkg install openal-soft:x64-windows

      - name: Configure CMake (MSVC)
        run: cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake" -DBUILD_TESTS=ON

      - name: Build
        run: cmake --build build --config Release

      - name: Run Tests
        run: |
          cd build
          ctest -C Release --output-on-failure --verbose
```

### Known Issues & Fixes
- **Profiler on Windows**: The `Profiler.cpp` contains extensive logic using `psapi.h`. This is currently simplified/disabled in the codebase to prevent header conflicts during cross-platform compilation. To enable full profiling on Windows, un-comment the logic in `engine/src/core/Profiler.cpp`.
- **Linker Errors (`optimized.lib`)**: If this recurs, ensure `USE_SYSTEM_SFML` is `ON` and that `vcpkg` is correctly providing the dependencies.
