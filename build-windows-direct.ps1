# PowerShell script to build the game directly with Visual Studio tools
# This script properly sets up the Visual Studio environment

Write-Host "Building codex-game with Visual Studio tools..." -ForegroundColor Green

# Find Visual Studio installation
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($vsPath) {
        Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Yellow
        
        # Set up Visual Studio environment
        $vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
        if (Test-Path $vcvarsPath) {
            Write-Host "Setting up Visual Studio environment..." -ForegroundColor Yellow
            
            # Install vcpkg if not present
            if (-not (Test-Path "C:\vcpkg\vcpkg.exe")) {
                Write-Host "Installing vcpkg..." -ForegroundColor Yellow
                git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
                & "C:\vcpkg\bootstrap-vcpkg.bat"
            }
            
            # Install SDL2 dependencies
            Write-Host "Installing SDL2 dependencies..." -ForegroundColor Yellow
            & "C:\vcpkg\vcpkg.exe" install sdl2:x64-windows sdl2-image:x64-windows
            
            # Build the game
            Write-Host "Building game..." -ForegroundColor Yellow
            $buildCommand = @"
call "$vcvarsPath" && cl /std:c++17 /I"C:\vcpkg\installed\x64-windows\include" src\main.cpp src\bitmap_font.cpp /link "C:\vcpkg\installed\x64-windows\lib\SDL2.lib" "C:\vcpkg\installed\x64-windows\lib\SDL2main.lib" "C:\vcpkg\installed\x64-windows\lib\SDL2_image.lib" /out:game.exe
"@
            
            cmd /c $buildCommand
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "Build successful!" -ForegroundColor Green
                
                # Copy SDL2 DLLs
                Write-Host "Copying SDL2 DLLs..." -ForegroundColor Yellow
                Copy-Item "C:\vcpkg\installed\x64-windows\bin\*.dll" -Destination "." -Force
                
                Write-Host "Game ready to run!" -ForegroundColor Green
            } else {
                Write-Host "Build failed!" -ForegroundColor Red
                exit 1
            }
        } else {
            Write-Host "vcvars64.bat not found at: $vcvarsPath" -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Visual Studio with C++ tools not found!" -ForegroundColor Red
        Write-Host "Please install Visual Studio Community with C++ development tools." -ForegroundColor Yellow
        exit 1
    }
} else {
    Write-Host "vswhere.exe not found. Please install Visual Studio." -ForegroundColor Red
    exit 1
}
