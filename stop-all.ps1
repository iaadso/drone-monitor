# Stop All Services Script
# Usage: .\stop-all.ps1

Write-Host "Stopping all Drone Monitoring services..." -ForegroundColor Yellow

# Stop Backend
$BackendProcess = Get-Process -Name "drone-backend" -ErrorAction SilentlyContinue
if ($BackendProcess) {
    Write-Host "  Stopping Backend (PID: $($BackendProcess.Id))..." -ForegroundColor Gray
    Stop-Process -Name "drone-backend" -Force
    Write-Host "  Backend stopped" -ForegroundColor Green
} else {
    Write-Host "  Backend not running" -ForegroundColor Gray
}

# Stop Simulator
$SimulatorProcess = Get-Process -Name "drone-simulator" -ErrorAction SilentlyContinue
if ($SimulatorProcess) {
    Write-Host "  Stopping Simulator (PID: $($SimulatorProcess.Id))..." -ForegroundColor Gray
    Stop-Process -Name "drone-simulator" -Force
    Write-Host "  Simulator stopped" -ForegroundColor Green
} else {
    Write-Host "  Simulator not running" -ForegroundColor Gray
}

# Check for Python HTTP server on port 8081
$Port8081 = netstat -ano | findstr ":8081" | findstr "LISTENING"
if ($Port8081) {
    $PID = ($Port8081 -split '\s+')[-1]
    Write-Host "  Stopping Frontend (PID: $PID)..." -ForegroundColor Gray
    Stop-Process -Id $PID -Force -ErrorAction SilentlyContinue
    Write-Host "  Frontend stopped" -ForegroundColor Green
} else {
    Write-Host "  Frontend not running" -ForegroundColor Gray
}

Write-Host ""
Write-Host "All services stopped" -ForegroundColor Green