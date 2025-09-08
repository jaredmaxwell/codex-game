@echo off
echo Building codex-game with Conan package manager...

REM Check if Python is available
python --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Python not found. Please install Python 3.7+ from https://python.org
    exit /b 1
)

REM Install Conan if not available
conan --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Installing Conan...
    pip install conan
)

REM Create conanfile.txt if it doesn't exist
if not exist "conanfile.txt" (
    echo Creating conanfile.txt...
    echo [requires] > conanfile.txt
    echo sdl/2.28.5 >> conanfile.txt
    echo sdl_image/2.8.2 >> conanfile.txt
    echo. >> conanfile.txt
    echo [generators] >> conanfile.txt
    echo CMakeDeps >> conanfile.txt
    echo CMakeToolchain >> conanfile.txt
)

REM Install dependencies
echo Installing dependencies...
conan install . --build=missing

REM Create CMakeLists.txt if it doesn't exist
if not exist "CMakeLists.txt" (
    echo Creating CMakeLists.txt...
    echo cmake_minimum_required(VERSION 3.15) > CMakeLists.txt
    echo project(codex_game) >> CMakeLists.txt
    echo. >> CMakeLists.txt
    echo set(CMAKE_CXX_STANDARD 17) >> CMakeLists.txt
    echo. >> CMakeLists.txt
    echo find_package(SDL2 REQUIRED) >> CMakeLists.txt
    echo find_package(SDL2_image REQUIRED) >> CMakeLists.txt
    echo. >> CMakeLists.txt
    echo add_executable(game src/main.cpp src/bitmap_font.cpp) >> CMakeLists.txt
    echo. >> CMakeLists.txt
    echo target_link_libraries(game SDL2::SDL2main SDL2::SDL2 SDL2_image::SDL2_image) >> CMakeLists.txt
)

REM Build with CMake
echo Building with CMake...
cmake --preset conan-default
cmake --build --preset conan-release

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
) else (
    echo Build failed!
    exit /b 1
)
