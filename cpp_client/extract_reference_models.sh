#!/bin/bash
# Extract reference OBJ models from testing/ archives.
# These large models are NOT stored in git — run this script after cloning
# to set up the reference models used by ProceduralShipGenerator as seed meshes.
#
# Requirements: unzip, 7z (p7zip-full on Linux, 7-Zip on Windows)
#
# Reference model inventory:
#   Intergalactic Spaceship (5MB OBJ)  — seed for frigates through cruisers
#   Vulcan Dkyr Class      (12MB OBJ)  — seed for battleships, capitals, titans
#   Modular Ship Modules   (< 1KB each) — already tracked in git under modules/

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TESTING_DIR="$SCRIPT_DIR/../testing"
OUT_DIR="$SCRIPT_DIR/assets/reference_models"

echo "=== Extracting reference OBJ models ==="

mkdir -p "$OUT_DIR"

# Intergalactic Spaceship (RAR)
if [ ! -f "$OUT_DIR/Intergalactic_Spaceship-(Wavefront).obj" ]; then
    if [ -f "$TESTING_DIR/99-intergalactic_spaceship-obj.rar" ]; then
        echo "Extracting Intergalactic Spaceship OBJ..."
        cd "$OUT_DIR"
        7z x "$TESTING_DIR/99-intergalactic_spaceship-obj.rar" -y > /dev/null
        echo "  ✓ Intergalactic_Spaceship-(Wavefront).obj"
    else
        echo "  ⚠ Skipping Intergalactic Spaceship (archive not found)"
    fi
else
    echo "  ✓ Intergalactic Spaceship already extracted"
fi

# Vulcan Dkyr Class (ZIP)
if [ ! -f "$OUT_DIR/Vulcan Dkyr Class/VulcanDKyrClass.obj" ]; then
    if [ -f "$TESTING_DIR/qy0sx26192io-VulcanDkyrClass.zip" ]; then
        echo "Extracting Vulcan Dkyr Class OBJ..."
        cd "$OUT_DIR"
        unzip -o "$TESTING_DIR/qy0sx26192io-VulcanDkyrClass.zip" > /dev/null
        echo "  ✓ Vulcan Dkyr Class/VulcanDKyrClass.obj"
    else
        echo "  ⚠ Skipping Vulcan Dkyr Class (archive not found)"
    fi
else
    echo "  ✓ Vulcan Dkyr Class already extracted"
fi

echo ""
echo "Reference models ready in: $OUT_DIR"
echo ""
echo "Inventory:"
find "$OUT_DIR" -name "*.obj" -exec ls -lh {} \; 2>/dev/null
