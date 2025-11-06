#!/bin/bash

# macOS run script for Black Hole Simulation
# This ensures the program runs from the correct directory to find shader files

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "Black Hole Simulation - macOS Launcher"
echo "=========================================="

# Check if executable exists
if [ ! -f "build/BlackHole3D" ]; then
    echo -e "${RED}[ERROR]${NC} Executable not found: build/BlackHole3D"
    echo "Please run ./build_macos.sh first to compile the project"
    exit 1
fi

# Check if shader files exist
SHADERS_OK=true
for shader in grid.vert grid.frag tonemap.frag geodesic_kerr.comp geodesic_schwarzschild.comp; do
    if [ ! -f "$shader" ]; then
        echo -e "${YELLOW}[WARNING]${NC} Shader file not found in project root: $shader"
        SHADERS_OK=false
    fi
done

if [ "$SHADERS_OK" = false ]; then
    echo -e "${YELLOW}[INFO]${NC} Some shader files are missing, but the program will try to find them"
fi

# Run from project root directory (so shader files can be found)
echo -e "${GREEN}[INFO]${NC} Running BlackHole3D from project root directory..."
echo ""

# Execute the program
./build/BlackHole3D

echo ""
echo "=========================================="
echo "Black Hole Simulation exited"
echo "=========================================="
