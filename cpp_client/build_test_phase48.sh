#!/bin/bash
# Build script for test_phase48_panels (Phase 4.8 D-Scan + Neocom + Module Rack)

echo "Building Phase 4.8 D-Scan / Neocom / Module Rack test..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DBUILD_TESTS=ON

# Build test_phase48_panels
make test_phase48_panels

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo "Run with: ./bin/test_phase48_panels"
    echo ""
    echo "Controls:"
    echo "  F1  - Toggle D-Scan panel"
    echo "  F2  - Toggle Neocom sidebar"
    echo "  V   - Trigger D-Scan"
    echo "  ESC - Exit"
else
    echo ""
    echo "Build failed!"
    exit 1
fi
