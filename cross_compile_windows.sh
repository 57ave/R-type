#!/bin/bash

# Exit on error
set -e

# Directory for the build
BUILD_DIR="build-windows"

# Clean build directory
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"

# Configure CMake with the MinGW toolchain
echo "Configuring CMake for Windows..."
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-MinGW-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DOPENAL_LIBRARY="$(pwd)/$BUILD_DIR/_deps/sfml-src/extlibs/libs-mingw/x64/libopenal32.a" \
    -DOPENAL_INCLUDE_DIR="$(pwd)/$BUILD_DIR/_deps/sfml-src/extlibs/headers/AL"

# Build the project
echo "Building project..."
cmake --build "$BUILD_DIR" -j$(nproc)

echo "Build complete. Check $BUILD_DIR for executables."
