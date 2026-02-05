#!/bin/bash
# Build script for Phase 4.6 advanced features test

set -e

echo "Building Phase 4.6 Advanced Features Test..."

# Create build directory
BUILD_DIR="build_test_phase46"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the test program
cmake --build . --target test_phase46_advanced -j$(nproc)

echo "Build complete!"
echo "Run with: ./$BUILD_DIR/bin/test_phase46_advanced"
