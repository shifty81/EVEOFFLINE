#!/bin/bash

# Build script for entity synchronization test
# Simple direct compilation without full CMake setup

echo "Building entity synchronization test..."

cd "$(dirname "$0")"

# Compile
g++ -std=c++17 \
    -I./include \
    test_entity_sync.cpp \
    src/core/entity.cpp \
    src/core/entity_manager.cpp \
    src/core/entity_message_parser.cpp \
    -o test_entity_sync \
    -lpthread

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo ""
    echo "Run with: ./test_entity_sync"
else
    echo "✗ Build failed!"
    exit 1
fi
