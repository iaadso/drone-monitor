# Start Drone Simulator Only
# Usage: .\start-simulator.ps1
# Note: Backend server must be running first (port 8080)

$ScriptDir = $PSScriptRoot
if (-not $ScriptDir) {
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
}
if (-not $ScriptDir) {
    $ScriptDir = Get-Location
}

$BuildDir = Join-Path $ScriptDir "build"
$SimulatorExe = Join-Path $BuildDir "simulator\drone-simulator.exe"

if (-not (Test-Path $SimulatorExe)) {
    Write-Host "[ERROR] Simulator executable not found: $SimulatorExe" -ForegroundColor Red
    Write-Host "Please compile the project first: cd build; mingw32-make" -ForegroundColor Yellow
    exit 1
}

# Check if backend is running
try {
    $Response = Invoke-WebRequest -Uri "http://localhost:8080/" -TimeoutSec 3 -UseBasicParsing
} catch {
    Write-Host "[ERROR] Backend server is not running!" -ForegroundColor Red
    Write-Host "Please start backend first: .\start-backend.ps1" -ForegroundColor Yellow
    exit 1
}

# Stop existing process
Get-Process -Name "drone-simulator" -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 1

Write-Host "Starting Drone Simulator..." -ForegroundColor Green
Write-Host "Target server: http://localhost:8080" -ForegroundColor Gray

Set-Location $BuildDir
& $SimulatorExe