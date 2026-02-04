#!/bin/bash
# Build test_lighting

set -e

echo "Building test_lighting..."

# Create build directory
mkdir -p cpp_client/build_test_lighting
cd cpp_client/build_test_lighting

# Run CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON

# Build the test
cmake --build . --target test_lighting -j$(nproc)

echo "Build complete!"
echo "Run with: ./cpp_client/build_test_lighting/bin/test_lighting"
