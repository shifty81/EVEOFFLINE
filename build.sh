#!/bin/bash
# Atlas - Unix Build Script
# Builds the C++ client and server using CMake

echo "================================================"
echo "Atlas - Build"
echo "================================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found!"
    echo "Please install CMake from https://cmake.org/download/"
    exit 1
fi

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "ERROR: No C++ compiler found!"
    echo "Install build-essential (Linux) or Xcode (macOS)"
    exit 1
fi

BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "Configuring CMake ($BUILD_TYPE)..."
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DUSE_SYSTEM_LIBS=ON

if [ $? -ne 0 ]; then
    echo ""
    echo "================================================"
    echo "CMAKE CONFIGURATION FAILED"
    echo "================================================"
    echo ""
    echo "Install dependencies:"
    echo "  Ubuntu/Debian: sudo apt-get install libgl1-mesa-dev libglew-dev libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev"
    echo "  macOS: brew install glfw glm glew nlohmann-json openal-soft"
    exit 1
fi

# Build
echo "Building..."
cmake --build . --config "$BUILD_TYPE" -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -eq 0 ]; then
    echo ""
    echo "================================================"
    echo "BUILD SUCCESSFUL"
    echo "================================================"
    echo ""
    echo "Executables:"
    find bin -type f -executable 2>/dev/null | head -10
    exit 0
else
    echo ""
    echo "================================================"
    echo "BUILD FAILED"
    echo "================================================"
    exit 1
fi
