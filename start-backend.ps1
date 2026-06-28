# Start Backend Server Only
# Usage: .\start-backend.ps1

$ScriptDir = $PSScriptRoot
if (-not $ScriptDir) {
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
}
if (-not $ScriptDir) {
    $ScriptDir = Get-Location
}

$BuildDir = Join-Path $ScriptDir "build"
$BackendExe = Join-Path $BuildDir "backend\drone-backend.exe"

Write-Host "ScriptDir: $ScriptDir" -ForegroundColor Gray
Write-Host "BuildDir: $BuildDir" -ForegroundColor Gray
Write-Host "BackendExe: $BackendExe" -ForegroundColor Gray

if (-not (Test-Path $BackendExe)) {
    Write-Host "[ERROR] Backend executable not found: $BackendExe" -ForegroundColor Red
    Write-Host "Please compile the project first: cd build; mingw32-make" -ForegroundColor Yellow
    exit 1
}

# Stop existing process
Get-Process -Name "drone-backend" -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 1

Write-Host "Starting Backend Server on http://localhost:8080" -ForegroundColor Green

Set-Location $BuildDir
& $BackendExe