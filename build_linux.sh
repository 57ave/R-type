#!/bin/bash
set -e

# Define vcpkg path
VCPKG_ROOT="./vcpkg"
VCPKG_EXE="$VCPKG_ROOT/vcpkg"

# Check if vcpkg directory exists
if [ ! -d "$VCPKG_ROOT" ]; then
    echo "[ERROR] vcpkg directory not found at $VCPKG_ROOT"
    exit 1
fi

# Bootstrap vcpkg if the executable doesn't exist
if [ ! -f "$VCPKG_EXE" ]; then
    echo "[INFO] Bootstrapping vcpkg..."
    "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
fi

# Install dependencies (optional here if using manifest mode in CMake, but good for explicit check)
# echo "[INFO] Installing dependencies..."
# "$VCPKG_EXE" install

# Create build directory
if [ -d "build" ]; then
    echo "[INFO] Cleaning build directory..."
    rm -rf build
fi
mkdir build

# Configure CMake
echo "[INFO] Configuring CMake..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Build
echo "[INFO] Building..."
cmake --build build --config Debug

echo "[SUCCESS] Build complete."
