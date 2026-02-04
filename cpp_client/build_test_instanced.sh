#!/bin/bash

# Build script for instanced rendering test

echo "Building Instanced Rendering Test..."

# Create build directory
mkdir -p build_test_instanced
cd build_test_instanced

# Compile and link test (this test doesn't need OpenGL, just logic testing)
g++ -std=c++17 -I../include -I../external/glm \
    ../test_instanced_rendering.cpp \
    -o test_instanced_rendering

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running tests..."
    ./test_instanced_rendering
else
    echo "Build failed!"
    exit 1
fi
