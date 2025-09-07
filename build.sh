#!/bin/bash

# Cross-platform build script for codex-game

echo "Building codex-game..."

# Detect operating system
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Detected macOS"
    if [ -f "Makefile.macos" ]; then
        make -f Makefile.macos
    else
        echo "Error: Makefile.macos not found"
        exit 1
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    echo "Detected Linux"
    if [ -f "Makefile.linux" ]; then
        make -f Makefile.linux
    else
        echo "Error: Makefile.linux not found"
        exit 1
    fi
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    # Windows (MSYS2/Cygwin)
    echo "Detected Windows (MSYS2/Cygwin)"
    if [ -f "Makefile" ]; then
        make
    else
        echo "Error: Makefile not found"
        exit 1
    fi
else
    echo "Unsupported operating system: $OSTYPE"
    exit 1
fi

echo "Build complete!"
