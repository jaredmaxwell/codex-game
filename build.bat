@echo off
echo Building codex-game for Windows...

REM Check if we're in MSYS2 environment
if exist "C:\msys64\usr\bin\bash.exe" (
    echo Using MSYS2 environment
    echo Current directory: %CD%
    
    REM Start MSYS2 terminal and run make
    start /wait C:\msys64\msys2_shell.cmd -defterm -here -no-start -mingw64 -c "make"
    
    if %ERRORLEVEL% EQU 0 (
        echo Build successful!
    ) else (
        echo Build failed!
        exit /b 1
    )
) else (
    echo MSYS2 not found. Please install MSYS2 and SDL2 dependencies.
    echo See BUILD.md for instructions.
    exit /b 1
)
