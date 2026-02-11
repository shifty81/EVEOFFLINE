#!/bin/bash
# Build script for EVE OFFLINE C++ Dedicated Server

set -e

echo "=================================="
echo "EVE OFFLINE Server Build Script"
echo "=================================="
echo ""

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed"
    echo "Please install CMake 3.15 or higher"
    exit 1
fi

# Parse arguments
USE_STEAM="ON"
BUILD_TYPE="Release"
CLEAN_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --no-steam)
            USE_STEAM="OFF"
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--no-steam] [--debug] [--clean]"
            exit 1
            ;;
    esac
done

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

echo "Configuration:"
echo "  Build Type: $BUILD_TYPE"
echo "  Steam SDK: $USE_STEAM"
echo ""

# Run CMake
echo "Running CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DUSE_STEAM_SDK=$USE_STEAM

echo ""
echo "Building..."

# Build
if command -v nproc &> /dev/null; then
    JOBS=$(nproc)
else
    JOBS=4
fi

make -j$JOBS

echo ""
echo "=================================="
echo "Build complete!"
echo "=================================="
echo ""
echo "Executable: build/bin/eve_dedicated_server"
echo "Tests: build/bin/test_systems"
echo "Config: build/bin/config/server.json"
echo ""
echo "To run the server:"
echo "  cd build/bin"
echo "  ./eve_dedicated_server"
echo ""
echo "To run tests:"
echo "  ./run_tests.sh"
echo ""
