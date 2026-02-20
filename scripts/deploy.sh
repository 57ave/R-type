#!/bin/bash
set -e

# Configuration
BUILD_DIR="build"
INSTALL_DIR="install"

echo "🚀 Starting R-Type Deployment..."

# 1. Configure
echo "🔧 Configuring CMake..."
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

# 2. Build
echo "🔨 Building Project..."
cmake --build "$BUILD_DIR" -j $(nproc)

# 3. Install
echo "📦 Installing to $INSTALL_DIR..."
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR"

# 4. Fix Missing Dependencies (SFML via CPM quirk)
echo "🔧 Fixing missing shared libraries..."
mkdir -p "$INSTALL_DIR/lib"
# Try to find SFML libs in build tree and copy them
find "$BUILD_DIR" -name "libsfml-*.so*" -exec cp -P {} "$INSTALL_DIR/lib/" \;

echo "✅ Deployment Complete!"
echo "Run the game with: cd $INSTALL_DIR/bin && ./r-type_game"
