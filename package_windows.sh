#!/bin/bash

# Configuration
BUILD_DIR="build-windows"
RELEASE_DIR="release-windows"
SFML_BIN_DIR="$BUILD_DIR/_deps/sfml-src/extlibs/bin/x64"

# Create release directory
if [ -d "$RELEASE_DIR" ]; then
    rm -rf "$RELEASE_DIR"
fi
mkdir "$RELEASE_DIR"

echo "Packaging release to $RELEASE_DIR..."

# 1. Copy Executables
echo "Copying executables..."
cp "$BUILD_DIR/game/r-type_game.exe" "$RELEASE_DIR/"
cp "$BUILD_DIR/server/r-type_server.exe" "$RELEASE_DIR/"
cp "$BUILD_DIR/client/r-type_client.exe" "$RELEASE_DIR/"

# 2. Copy System DLLs (Plugins)
echo "Copying Game System plugins..."
cp "$BUILD_DIR"/engine/*.dll "$RELEASE_DIR/"

# 3. Copy Dependencies (OpenAL)
# Note: Static linking of libgcc/libstdc++ handled in build script.
# OpenAL is usually dynamic.
if [ -f "$SFML_BIN_DIR/openal32.dll" ]; then
    echo "Copying openal32.dll..."
    cp "$SFML_BIN_DIR/openal32.dll" "$RELEASE_DIR/"
else
    echo "WARNING: openal32.dll not found in expected location: $SFML_BIN_DIR"
fi

# 3.5 Copy MinGW Runtime DLLs
echo "Copying MinGW runtime DLLs..."
cp /usr/lib/gcc/x86_64-w64-mingw32/14-posix/libstdc++-6.dll "$RELEASE_DIR/"
cp /usr/lib/gcc/x86_64-w64-mingw32/14-posix/libgcc_s_seh-1.dll "$RELEASE_DIR/"
cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll "$RELEASE_DIR/"


# 4. Copy Assets
echo "Copying assets..."
mkdir -p "$RELEASE_DIR/assets"
if [ -d "assets" ]; then
    cp -r "assets/"* "$RELEASE_DIR/assets/"
fi
if [ -d "client/assets" ]; then
    cp -r "client/assets/"* "$RELEASE_DIR/assets/"
fi

echo "Packaging complete!"
echo "Please transfer the contents of '$RELEASE_DIR' to your Windows machine."
