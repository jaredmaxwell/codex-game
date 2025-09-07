@echo off
echo Building codex-game for Windows...

REM Simple approach - just run make directly if we're already in MSYS2 environment
REM or provide instructions for manual building

echo.
echo To build the game, please:
echo 1. Open MSYS2 terminal (MinGW 64-bit)
echo 2. Navigate to this directory
echo 3. Run: make
echo.
echo Or run this command in MSYS2:
echo cd /c/Users/Swift/CLionProjects/codex-game/codex-game
echo make
echo.

REM Try to run make directly (will work if in MSYS2 environment)
make

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
) else (
    echo.
    echo Make failed. Please ensure you are in an MSYS2 environment.
    echo See BUILD.md for detailed instructions.
)
