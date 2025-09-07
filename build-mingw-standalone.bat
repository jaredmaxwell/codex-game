@echo off
echo Building codex-game with standalone MinGW-w64...

REM Set paths for MinGW-w64 (adjust these paths as needed)
set MINGW_PATH=C:\mingw64
set SDL2_PATH=C:\SDL2

REM Check if MinGW-w64 is available
if not exist "%MINGW_PATH%\bin\g++.exe" (
    echo MinGW-w64 not found at %MINGW_PATH%
    echo Please download from: https://www.mingw-w64.org/downloads/
    echo Or use the installer from: https://github.com/niXman/mingw-builds-binaries/releases
    exit /b 1
)

REM Check if SDL2 is available
if not exist "%SDL2_PATH%\include\SDL2\SDL.h" (
    echo SDL2 not found at %SDL2_PATH%
    echo Please download SDL2 development libraries from: https://www.libsdl.org/download-2.0.php
    echo Extract to %SDL2_PATH%
    exit /b 1
)

REM Add MinGW-w64 to PATH
set PATH=%MINGW_PATH%\bin;%PATH%

REM Build the game
echo Building...
g++ -std=c++17 -I"%SDL2_PATH%\include" -I"%SDL2_PATH%\include\SDL2" src\main.cpp -L"%SDL2_PATH%\lib\x64" -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -o game.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Copying SDL2 DLLs...
    copy "%SDL2_PATH%\lib\x64\*.dll" .
    echo Game ready to run!
) else (
    echo Build failed!
    exit /b 1
)
