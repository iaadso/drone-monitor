# Start Frontend Web Server Only
# Usage: .\start-frontend.ps1

$ScriptDir = $PSScriptRoot
if (-not $ScriptDir) {
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
}
if (-not $ScriptDir) {
    $ScriptDir = Get-Location
}

$FrontendDir = Join-Path $ScriptDir "frontend"

if (-not (Test-Path $FrontendDir)) {
    Write-Host "[ERROR] Frontend directory not found: $FrontendDir" -ForegroundColor Red
    exit 1
}

# Check Python availability
$PythonCmd = Get-Command "python" -ErrorAction SilentlyContinue
if (-not $PythonCmd) {
    Write-Host "[ERROR] Python not found" -ForegroundColor Red
    Write-Host "Please install Python or add it to PATH" -ForegroundColor Yellow
    exit 1
}

Write-Host "Starting Frontend Web Server on http://localhost:8081" -ForegroundColor Green

Set-Location $FrontendDir
& python -m http.server 8081