#!/bin/bash
# Build script for server response handling test

set -e  # Exit on error

echo "Building Server Response Handling Test..."

# Create build directory
mkdir -p build_test_responses
cd build_test_responses

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the test
cmake --build . --target test_server_responses

echo ""
echo "Build complete! Running test..."
echo ""

# Run the test
./test_server_responses

echo ""
echo "Test complete!"
