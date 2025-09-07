# PowerShell build script for codex-game

Write-Host "Building codex-game for Windows..." -ForegroundColor Green

# Check if MSYS2 is available
if (Test-Path "C:\msys64\usr\bin\bash.exe") {
    Write-Host "MSYS2 found. Building with MSYS2..." -ForegroundColor Yellow
    
    # Get current directory and convert to MSYS2 path format
    $currentDir = Get-Location
    $msysPath = $currentDir.Path -replace "C:", "/c" -replace "\\", "/"
    
    Write-Host "Current directory: $currentDir" -ForegroundColor Cyan
    Write-Host "MSYS2 path: $msysPath" -ForegroundColor Cyan
    
    # Run make in MSYS2 environment
    $makeCommand = "cd '$msysPath' && make"
    $result = & "C:\msys64\usr\bin\bash.exe" -c $makeCommand
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build successful!" -ForegroundColor Green
    } else {
        Write-Host "Build failed!" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "MSYS2 not found. Please install MSYS2 and SDL2 dependencies." -ForegroundColor Red
    Write-Host "See BUILD.md for instructions." -ForegroundColor Yellow
    exit 1
}
