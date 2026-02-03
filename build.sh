#!/bin/bash
# EVE OFFLINE - Unix Build and Test Script
# Quick access script for Linux/macOS users

echo "================================================"
echo "EVE OFFLINE - Build and Test"
echo "================================================"
echo ""

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 not found!"
    echo "Please install Python 3.11 or higher"
    exit 1
fi

# Run the build script
python3 build_and_test.py "$@"

# Check result
if [ $? -eq 0 ]; then
    echo ""
    echo "================================================"
    echo "BUILD SUCCESSFUL"
    echo "================================================"
    exit 0
else
    echo ""
    echo "================================================"
    echo "BUILD FAILED"
    echo "================================================"
    exit 1
fi
