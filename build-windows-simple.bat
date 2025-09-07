@echo off
echo Building codex-game with Visual Studio tools...

REM Install vcpkg if not present
if not exist "C:\vcpkg\vcpkg.exe" (
    echo Installing vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
    C:\vcpkg\bootstrap-vcpkg.bat
)

REM Install SDL2 dependencies
echo Installing SDL2 dependencies...
C:\vcpkg\vcpkg.exe install sdl2:x64-windows sdl2-ttf:x64-windows sdl2-image:x64-windows

REM Find Visual Studio and set up environment
echo Setting up Visual Studio environment...
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VS_PATH=%%i

if "%VS_PATH%"=="" (
    echo Visual Studio not found!
    exit /b 1
)

set VC_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat

REM Build the game
echo Building game...
call "%VC_PATH%" && cl /std:c++17 /I"C:\vcpkg\installed\x64-windows\include" src\main.cpp /link "C:\vcpkg\installed\x64-windows\lib\SDL2.lib" "C:\vcpkg\installed\x64-windows\lib\SDL2main.lib" "C:\vcpkg\installed\x64-windows\lib\SDL2_ttf.lib" "C:\vcpkg\installed\x64-windows\lib\SDL2_image.lib" /out:game.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Copying SDL2 DLLs...
    copy "C:\vcpkg\installed\x64-windows\bin\*.dll" .
    echo Game ready to run!
) else (
    echo Build failed!
    exit /b 1
)
