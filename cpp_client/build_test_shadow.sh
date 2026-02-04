#!/bin/bash

# Build shadow mapping test

BUILD_DIR="build_test_shadow"

echo "Building Shadow Mapping Test..."

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make test_shadow_mapping

echo ""
echo "Build complete!"
echo "Run with: ./$BUILD_DIR/bin/test_shadow_mapping"
