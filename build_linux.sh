#!/bin/bash
set -e

# ============================================
# R-Type Linux Build Script
# ============================================
# Usage: ./build_linux.sh [debug|release] [--install] [--clean]
#
# Options:
#   debug     Build in Debug mode (default)
#   release   Build in Release mode
#   --install Install binaries to release_linux/
#   --clean   Clean build directory before building
# ============================================

BUILD_TYPE="debug"
DO_INSTALL=false
DO_CLEAN=false

for arg in "$@"; do
    case "$arg" in
        release) BUILD_TYPE="release" ;;
        debug)   BUILD_TYPE="debug" ;;
        --install) DO_INSTALL=true ;;
        --clean)   DO_CLEAN=true ;;
        *)
            echo "[ERROR] Unknown argument: $arg"
            echo "Usage: $0 [debug|release] [--install] [--clean]"
            exit 1
            ;;
    esac
done

PRESET_NAME="linux-${BUILD_TYPE}"
BUILD_DIR="build/${PRESET_NAME}"
INSTALL_DIR="release_linux"

echo "============================================"
echo " R-Type Linux Build"
echo " Preset: ${PRESET_NAME}"
echo "============================================"

# ============================================
# 1. Install system dependencies
# ============================================
install_dependencies() {
    echo "[INFO] Checking for system dependencies..."

    if command -v apt-get &> /dev/null; then
        echo "[INFO] Detected Debian/Ubuntu based system."

        PACKAGES=(
            # Build tools
            build-essential cmake ninja-build git pkg-config

            # X11 / Window system
            libxrandr-dev libxcursor-dev libxi-dev

            # Input / Device
            libudev-dev

            # Audio
            libopenal-dev libflac-dev libvorbis-dev libogg-dev

            # Graphics
            libgl1-mesa-dev libglu1-mesa-dev libfreetype-dev

            # Lua (optional, CPM fallback available)
            liblua5.4-dev
        )

        # Check if all packages are already installed
        MISSING=()
        for pkg in "${PACKAGES[@]}"; do
            if ! dpkg -s "$pkg" &> /dev/null; then
                MISSING+=("$pkg")
            fi
        done

        if [ ${#MISSING[@]} -eq 0 ]; then
            echo "[INFO] All dependencies are already installed."
        else
            echo "[INFO] Installing missing packages:"
            printf "  - %s\n" "${MISSING[@]}"
            sudo apt-get update
            sudo apt-get install -y "${MISSING[@]}"
        fi

    elif command -v dnf &> /dev/null; then
        echo "[INFO] Detected Fedora/RHEL based system."
        sudo dnf install -y \
            gcc-c++ cmake ninja-build git pkgconf-pkg-config \
            libXrandr-devel libXcursor-devel libXi-devel \
            systemd-devel \
            openal-soft-devel flac-devel libvorbis-devel libogg-devel \
            mesa-libGL-devel mesa-libGLU-devel freetype-devel \
            lua-devel

    elif command -v pacman &> /dev/null; then
        echo "[INFO] Detected Arch based system."
        sudo pacman -S --needed --noconfirm \
            base-devel cmake ninja git pkgconf \
            libxrandr libxcursor libxi \
            openal flac libvorbis libogg \
            mesa glu freetype2 \
            lua

    else
        echo "[WARNING] Unsupported package manager."
        echo "Please install the following manually:"
        echo "  - C++ compiler (g++ or clang++)"
        echo "  - cmake, ninja-build, git, pkg-config"
        echo "  - X11 dev libs (xrandr, xcursor, xi)"
        echo "  - libudev-dev, libopenal-dev, libflac-dev, libvorbis-dev"
        echo "  - libgl1-mesa-dev, libfreetype-dev, liblua5.4-dev"
    fi
}

install_dependencies

# ============================================
# 2. Clean if requested
# ============================================
if [ "$DO_CLEAN" = true ] && [ -d "$BUILD_DIR" ]; then
    echo "[INFO] Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# ============================================
# 3. Configure with CMake preset
# ============================================
echo "[INFO] Configuring CMake (preset: ${PRESET_NAME})..."
cmake --preset "$PRESET_NAME"

# ============================================
# 4. Build
# ============================================
echo "[INFO] Building ($(nproc) jobs)..."
cmake --build --preset "$PRESET_NAME" -j "$(nproc)"

echo "[SUCCESS] Build complete: $BUILD_DIR"

# ============================================
# 5. Install (optional)
# ============================================
if [ "$DO_INSTALL" = true ]; then
    echo "[INFO] Installing to $INSTALL_DIR..."
    cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR"
    echo "[SUCCESS] Installation complete: $INSTALL_DIR/"
    echo "-------------------------------------------------------"
    echo "  Run: ./$INSTALL_DIR/bin/r-type_server"
    echo "  Run: ./$INSTALL_DIR/bin/game"
    echo "-------------------------------------------------------"
    ls -R "$INSTALL_DIR/" 2>/dev/null || true
fi
