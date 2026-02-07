#!/bin/bash
# Build script for Star Map Demo

echo "Building EVE OFFLINE Star Map Demo..."

# Create build directory
mkdir -p build_starmap_demo
cd build_starmap_demo

# Compile source files
g++ -std=c++17 -c ../src/ui/star_map.cpp -I../include -I../external -o star_map.o
g++ -std=c++17 -c ../src/core/ship_physics.cpp -I../include -I../external -o ship_physics.o
g++ -std=c++17 -c ../src/rendering/camera.cpp -I../include -I../external -o camera.o

# Compile test program
g++ -std=c++17 -c ../test_starmap_demo.cpp -I../include -I../external -o test_starmap_demo.o

# Link
g++ -o starmap_demo test_starmap_demo.o star_map.o ship_physics.o camera.o \
    -lGL -lGLEW -lglfw -lm

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./build_starmap_demo/starmap_demo"
else
    echo "Build failed!"
    exit 1
fi
