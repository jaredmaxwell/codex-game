#!/bin/bash

echo "Building Codex Game for WebAssembly..."

# Check if Emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten not found. Please install Emscripten first."
    echo "Visit: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

# Create build directory
mkdir -p build-web
cd build-web

# Configure with Emscripten
echo "Configuring with Emscripten..."
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed"
    exit 1
fi

# Build
echo "Building..."
emmake make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Error: Build failed"
    exit 1
fi

echo ""
echo "Build successful!"
echo "Open build-web/game.html in your web browser to play."
echo ""
