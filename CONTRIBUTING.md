# Contributing to R-Type

Thank you for your interest in contributing to the R-Type project! This guide will help you get started with development.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Building the Project](#building-the-project)
  - [Linux](#linux)
  - [Windows](#windows)
  - [Cross-Compiling for Windows](#cross-compiling-for-windows)
- [Running Tests](#running-tests)
- [Code Formatting](#code-formatting)
- [Submitting Changes](#submitting-changes)
- [CI Pipeline](#ci-pipeline)

## Prerequisites

### Linux

Install the required dependencies:

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libasound2-dev \
    libpulse-dev \
    libudev-dev \
    libopenal-dev \
    libvorbis-dev \
    libflac-dev \
    libfreetype6-dev \
    python3 \
    clang-format
```

### Windows

Install:
- **Visual Studio 2022** (with C++ development tools)
- **CMake** (version 3.20+)
- **vcpkg** for dependency management

## Building the Project

### Linux

#### Using GCC (default)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(nproc)
```

#### Using Clang

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++
cmake --build build -j $(nproc)
```

### Windows

Using Visual Studio Developer Command Prompt:

```cmd
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

### Cross-Compiling for Windows

From Linux using MinGW:

```bash
sudo apt-get install g++-mingw-w64
chmod +x scripts/build_windows.sh
./scripts/build_windows.sh
```

## Running Tests

After building, run the test suite:

```bash
cd build
ctest --output-on-failure --verbose
```

To run specific tests:

```bash
cd build
./tests/BasicNetworkTests
```

## Code Formatting

This project uses **clang-format** with a Google-based style. All code must be properly formatted before submission.

### Format All Files

```bash
./scripts/format.sh
```

### Check Formatting (without modifying files)

```bash
./scripts/format.sh --check
```

**Note:** The CI pipeline will automatically check formatting. Unformatted code will fail the build.

## Submitting Changes

### Commit Convention

Use conventional commits:

- `feat:` - New features
- `fix:` - Bug fixes
- `docs:` - Documentation changes
- `style:` - Code style/formatting changes
- `refactor:` - Code refactoring
- `perf:` - Performance improvements
- `test:` - Test additions/changes
- `chore:` - Build system, dependencies, etc.

Example:
```
feat(network): add UDP packet compression
fix(rendering): resolve texture memory leak
docs: update build instructions for macOS
```

### Pull Request Process

1. **Create a feature branch**:
   ```bash
   git checkout -b feat/your-feature-name
   ```

2. **Make your changes** and commit:
   ```bash
   git add .
   git commit -m "feat: add new feature"
   ```

3. **Format your code**:
   ```bash
   ./scripts/format.sh
   ```

4. **Run tests**:
   ```bash
   cd build && ctest
   ```

5. **Push to GitHub**:
   ```bash
   git push origin feat/your-feature-name
   ```

6. **Open a Pull Request** on GitHub

### PR Requirements

- ✅ All CI checks must pass (GCC, Clang, MSVC builds)
- ✅ Code must be properly formatted
- ✅ Tests must pass
- ✅ Include a clear description of changes
- ✅ Reference related issues if applicable

## CI Pipeline

For detailed information about our workflows, including the disabled Windows build configuration, please refer to the [CI/CD Documentation](docs/CICD.md).

### Jobs

1. **Linux (GCC)** - Builds with GCC and runs tests
2. **Linux (Clang)** - Builds with Clang and runs tests
3. **Code Formatting** - Validates code formatting

All jobs must pass before a PR can be merged.

### Local Testing

Before pushing, you can verify your changes locally:

```bash
# Build with GCC
cmake -B build_gcc -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build_gcc

# Build with Clang
cmake -B build_clang -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build_clang

# Check formatting
./scripts/format.sh --check

# Run tests
cd build_gcc && ctest
```

## Getting Help

- Open an issue on GitHub for bugs or feature requests
- Check existing documentation in the `docs/` folder
- Review the [README.md](README.md) for project overview

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.
