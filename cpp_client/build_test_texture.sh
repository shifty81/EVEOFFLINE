#!/bin/bash

# Build script for texture loading test

echo "Building Texture Loading Test..."

# Create build directory
mkdir -p build_test_texture
cd build_test_texture

# Compile and link test
g++ -std=c++17 -I../include -I../external -I../external/stb \
    ../test_texture_loading.cpp \
    -o test_texture_loading

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running tests..."
    ./test_texture_loading
else
    echo "Build failed!"
    exit 1
fi
