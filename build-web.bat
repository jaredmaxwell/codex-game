@echo off
echo Building Codex Game for WebAssembly...

REM Check if Emscripten is installed
where emcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: Emscripten not found. Please install Emscripten first.
    echo Visit: https://emscripten.org/docs/getting_started/downloads.html
    pause
    exit /b 1
)

REM Create build directory
if not exist build-web mkdir build-web
cd build-web

REM Configure with Emscripten
echo Configuring with Emscripten...
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake configuration failed
    pause
    exit /b 1
)

REM Build
echo Building...
emmake make -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% NEQ 0 (
    echo Error: Build failed
    pause
    exit /b 1
)

echo.
echo Build successful! 
echo Open build-web/game.html in your web browser to play.
echo.
pause
