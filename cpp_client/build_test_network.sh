#!/bin/bash
# Build network test

echo "Building network test..."

# Create build directory
mkdir -p build_test_network
cd build_test_network

# Run CMake
cmake .. -DBUILD_TESTS=ON -DUSE_SYSTEM_LIBS=ON

# Build only the test
cmake --build . --target test_network

echo "Build complete. Binary: build_test_network/test_network"
