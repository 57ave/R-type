#!/bin/bash
# Download Sol3 header-only library

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENGINE_DIR="$(dirname "$SCRIPT_DIR")"
SOL3_DIR="$ENGINE_DIR/external/sol3"

echo "🔽 Downloading Sol3 (Lua C++ binding)..."
echo ""

# Create directory
mkdir -p "$SOL3_DIR/sol"

# Download Sol3 single header
SOL3_VERSION="v3.3.0"
SOL3_URL="https://github.com/ThePhD/sol2/releases/download/${SOL3_VERSION}/sol.hpp"

echo "Downloading from: $SOL3_URL"
echo "Installing to: $SOL3_DIR/sol/sol.hpp"
echo ""

if command -v curl &> /dev/null; then
    curl -L "$SOL3_URL" -o "$SOL3_DIR/sol/sol.hpp"
elif command -v wget &> /dev/null; then
    wget "$SOL3_URL" -O "$SOL3_DIR/sol/sol.hpp"
else
    echo "❌ Error: Neither curl nor wget found!"
    echo "Please install curl or wget, or download manually:"
    echo "  $SOL3_URL"
    echo "  → $SOL3_DIR/sol/sol.hpp"
    exit 1
fi

# Verify download
if [ -f "$SOL3_DIR/sol/sol.hpp" ]; then
    SIZE=$(stat -f%z "$SOL3_DIR/sol/sol.hpp" 2>/dev/null || stat -c%s "$SOL3_DIR/sol/sol.hpp" 2>/dev/null)
    echo ""
    echo "✅ Sol3 downloaded successfully!"
    echo "   File size: $SIZE bytes"
    echo "   Location: $SOL3_DIR/sol/sol.hpp"
    echo ""
    echo "Sol3 version: $SOL3_VERSION"
    echo "License: MIT"
    echo "Repository: https://github.com/ThePhD/sol2"
else
    echo ""
    echo "❌ Download failed!"
    echo "Please download manually from:"
    echo "  https://github.com/ThePhD/sol2/releases"
    exit 1
fi

echo ""
echo "🎉 Ready to build!"
echo "   cd build && cmake .."
