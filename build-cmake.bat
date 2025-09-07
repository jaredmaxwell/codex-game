@echo off
echo Building codex-game with CMake...

REM Check if CMake is available
cmake --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo CMake not found. Please install CMake from https://cmake.org/download/
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    echo Trying with MinGW Makefiles...
    cmake .. -G "MinGW Makefiles"
    if %ERRORLEVEL% NEQ 0 (
        echo CMake configuration failed with both generators!
        exit /b 1
    )
)

REM Build the project
echo Building...
cmake --build . --config Release

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable created in build/Release/ or build/
) else (
    echo Build failed!
    exit /b 1
)
