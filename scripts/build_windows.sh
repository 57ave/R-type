#!/bin/bash
set -e

BUILD_DIR="build_windows"
TOOLCHAIN_FILE="cmake/Toolchain-MinGW.cmake"

echo "🚀 Starting R-Type Windows Cross-Compilation..."

if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "❌ Error: MinGW-w64 compiler not found!"
    echo "Please install it: sudo apt install g++-mingw-w64"
    exit 1
fi

echo "🔧 Configuring CMake with MinGW Toolchain..."
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -Dsfml_DIR=""

echo "🔨 Building Project (Windows)..."
cmake --build "$BUILD_DIR" -j $(nproc)

echo "📦 Packaging Windows Distribution..."
INSTALL_DIR="release_windows"
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR"

echo "✅ Windows Build & Packaging Complete!"
echo "-------------------------------------------------------"
echo "Your game is ready in: $INSTALL_DIR/"
echo "Instructions for launch:"
echo "1. Transfer the '$INSTALL_DIR/' folder to a Windows machine."
echo "2. Run $INSTALL_DIR/bin/r-type_server.exe"
echo "3. Run $INSTALL_DIR/bin/r-type_game.exe"
echo "-------------------------------------------------------"

ls -R "$INSTALL_DIR/"
