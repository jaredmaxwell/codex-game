# Build Instructions for codex-game

This document provides instructions for building the game on different platforms.

## Prerequisites

### Windows (MSYS2)
```bash
# Install MSYS2 from https://www.msys2.org/
# Then install SDL2 dependencies:
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_image
```

### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install SDL2 dependencies
brew install sdl2 sdl2_ttf sdl2_image
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
```

### Linux (Fedora/CentOS)
```bash
sudo dnf install SDL2-devel SDL2_ttf-devel SDL2_image-devel
```

## Building

### Manual Build (Recommended)

**Windows (MSYS2):**
```bash
make
```

**macOS:**
```bash
make -f Makefile.macos
```

**Linux:**
```bash
make -f Makefile.linux
```

### Automatic Build Scripts

**Windows:**
```cmd
# Simple batch file (provides instructions)
build-simple.bat

# PowerShell script (if execution policy allows)
.\build.ps1
```

**macOS/Linux:**
```bash
chmod +x build.sh
./build.sh
```

**Note:** The manual `make` command is the most reliable method on Windows.

## Cross-Compilation

### Using GitHub Actions (Recommended)
The easiest way to build for multiple platforms is using GitHub Actions:

1. Push your code to a GitHub repository
2. The `.github/workflows/build.yml` file will automatically build for:
   - Windows (x64)
   - macOS (x64/ARM64)
   - Linux (x64)

3. Download the built binaries from the Actions tab

### Local Cross-Compilation

Cross-compiling from Windows to macOS is complex due to Apple's toolchain requirements. The recommended approaches are:

1. **Use GitHub Actions** (easiest)
2. **Use a macOS virtual machine**
3. **Use cloud-based macOS services**

## Assets

The game supports custom assets in the `assets/` directory:

- `char.png` - Player character sprite
- `enemy1.png` to `enemy10.png` - Enemy sprites for each level

If assets are not found, the game will use colored rectangles as placeholders.

## Troubleshooting

### Windows
- Ensure MSYS2 is properly installed and in PATH
- Make sure all SDL2 dependencies are installed via pacman

### macOS
- Ensure Homebrew is installed and up to date
- Check that SDL2 libraries are in the correct paths

### Linux
- Ensure development packages are installed (not just runtime)
- Check that pkg-config can find SDL2 libraries

## Output

The build process will create an executable named `game` (or `game.exe` on Windows) in the project root directory.
