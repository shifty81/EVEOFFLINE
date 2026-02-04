#!/bin/bash
# Build test_asteroid_field

set -e

echo "Building test_asteroid_field..."

# Create build directory
mkdir -p cpp_client/build_test_asteroid
cd cpp_client/build_test_asteroid

# Run CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON

# Build the test
cmake --build . --target test_asteroid_field -j$(nproc)

echo "Build complete!"
echo "Run with: ./cpp_client/build_test_asteroid/bin/test_asteroid_field"
