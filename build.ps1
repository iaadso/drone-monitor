# Build Script - Compile all C++ components
# Usage: .\build.ps1

$ScriptDir = $PSScriptRoot
if (-not $ScriptDir) {
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
}
if (-not $ScriptDir) {
    $ScriptDir = Get-Location
}

$BuildDir = Join-Path $ScriptDir "build"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Drone Monitoring System Build" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Check CMake availability
$CMakeCmd = Get-Command "cmake" -ErrorAction SilentlyContinue
if (-not $CMakeCmd) {
    Write-Host "[ERROR] CMake not found" -ForegroundColor Red
    Write-Host "Please install CMake: choco install cmake -y" -ForegroundColor Yellow
    exit 1
}

# Check Make availability
$MakeCmd = Get-Command "mingw32-make" -ErrorAction SilentlyContinue
if (-not $MakeCmd) {
    Write-Host "[ERROR] mingw32-make not found" -ForegroundColor Red
    Write-Host "Please install MinGW: choco install mingw -y" -ForegroundColor Yellow
    exit 1
}

# Create build directory if not exists
if (-not (Test-Path $BuildDir)) {
    Write-Host "[1/3] Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
} else {
    Write-Host "[1/3] Build directory exists, cleaning..." -ForegroundColor Yellow
    Get-Process -Name "drone-backend" -ErrorAction SilentlyContinue | Stop-Process -Force
    Get-Process -Name "drone-simulator" -ErrorAction SilentlyContinue | Stop-Process -Force
}

Set-Location $BuildDir

# Generate Makefiles with CMake
Write-Host "[2/3] Generating Makefiles with CMake..." -ForegroundColor Yellow
& cmake "$ScriptDir" -G "MinGW Makefiles"
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] CMake failed" -ForegroundColor Red
    exit 1
}

# Compile with Make
Write-Host "[3/3] Compiling project..." -ForegroundColor Yellow
& mingw32-make -j4
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Build failed" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Build completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executables:" -ForegroundColor White
Write-Host "  Backend:   $BuildDir\backend\drone-backend.exe" -ForegroundColor Yellow
Write-Host "  Simulator: $BuildDir\simulator\drone-simulator.exe" -ForegroundColor Yellow
Write-Host ""
Write-Host "To start the system, run: .\start-all.ps1" -ForegroundColor Cyan