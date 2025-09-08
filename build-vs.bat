@echo off
echo Building codex-game with Visual Studio and vcpkg...

REM Check if vcpkg is available
if not exist "C:\vcpkg\vcpkg.exe" (
    echo vcpkg not found. Installing vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
    cd C:\vcpkg
    .\bootstrap-vcpkg.bat
    .\vcpkg integrate install
    cd /d "%~dp0"
)

REM Install SDL2 dependencies via vcpkg
echo Installing SDL2 dependencies...
C:\vcpkg\vcpkg.exe install sdl2:x64-windows sdl2-image:x64-windows

REM Build with Visual Studio
echo Building with Visual Studio...
cl /std:c++17 /I"C:\vcpkg\installed\x64-windows\include" src\main.cpp src\bitmap_font.cpp /link "C:\vcpkg\installed\x64-windows\lib\SDL2.lib" "C:\vcpkg\installed\x64-windows\lib\SDL2main.lib" "C:\vcpkg\installed\x64-windows\lib\SDL2_image.lib" /out:game.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
) else (
    echo Build failed!
    exit /b 1
)
