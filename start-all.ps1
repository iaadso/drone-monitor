# Drone Monitoring System - One-click Start Script
# Usage: Run this script to start/stop/status all components

$ErrorActionPreference = "Continue"

$ScriptDir = $PSScriptRoot
if (-not $ScriptDir) {
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
}
if (-not $ScriptDir) {
    $ScriptDir = Get-Location
}

$BuildDir = Join-Path $ScriptDir "build"
$FrontendDir = Join-Path $ScriptDir "frontend"
$PidFile = Join-Path $BuildDir "dms.pids"

function Write-Status($text, $color = "White") {
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] $text" -ForegroundColor $color
}

function Save-Pids {
    $pids = @{
        backend = if ($script:BackendProcess) { $script:BackendProcess.Id } else { 0 }
        simulator = if ($script:SimulatorProcess) { $script:SimulatorProcess.Id } else { 0 }
        frontend = if ($script:FrontendProcess) { $script:FrontendProcess.Id } else { 0 }
    }
    $pids | ConvertTo-Json | Out-File -FilePath $PidFile -Encoding utf8
}

function Load-Pids {
    if (Test-Path $PidFile) {
        return Get-Content $PidFile | ConvertFrom-Json
    }
    return $null
}

function Stop-ServiceByProcessId($processId, $name) {
    if ($processId -and $processId -gt 0) {
        try {
            $proc = Get-Process -Id $processId -ErrorAction Stop
            Write-Status "Stopping $name (PID: $processId)..." "Yellow"
            $proc | Stop-Process -Force
            Start-Sleep -Milliseconds 500
            Write-Status "$name stopped" "Green"
        } catch {
            Write-Status "$name (PID: $processId) not running or already stopped" "Gray"
        }
    }
}

function Stop-All {
    Write-Status "Stopping all services..." "Yellow"
    $pids = Load-Pids
    if ($pids) {
        Stop-ServiceByProcessId $pids.backend "Backend"
        Stop-ServiceByProcessId $pids.simulator "Simulator"
        Stop-ServiceByProcessId $pids.frontend "Frontend"
        Remove-Item -Path $PidFile -Force -ErrorAction SilentlyContinue
    } else {
        Get-Process -Name "drone-backend" -ErrorAction SilentlyContinue | Stop-Process -Force
        Get-Process -Name "drone-simulator" -ErrorAction SilentlyContinue | Stop-Process -Force
        $Port8081 = netstat -ano | findstr ":8081" | findstr "LISTENING"
        if ($Port8081) {
            $PID = ($Port8081 -split '\s+')[-1]
            Stop-Process -Id $PID -Force -ErrorAction SilentlyContinue
        }
    }
    Write-Status "All services stopped" "Green"
}

function Get-ServiceStatus {
    Write-Status "Service Status:" "Cyan"
    
    $backendRunning = Get-Process -Name "drone-backend" -ErrorAction SilentlyContinue
    $simulatorRunning = Get-Process -Name "drone-simulator" -ErrorAction SilentlyContinue
    
    $frontendRunning = $false
    $frontendPid = 0
    $Port8081 = netstat -ano | findstr ":8081" | findstr "LISTENING"
    if ($Port8081) {
        $frontendPid = ($Port8081 -split '\s+')[-1]
        $frontendRunning = $true
    }
    
    if ($backendRunning) {
        Write-Status "Backend:        RUNNING (PID: $($backendRunning.Id), http://localhost:8080)" "Green"
    } else {
        Write-Status "Backend:        STOPPED" "Red"
    }
    
    if ($simulatorRunning) {
        Write-Status "Simulator:      RUNNING (PID: $($simulatorRunning.Id))" "Green"
    } else {
        Write-Status "Simulator:      STOPPED" "Red"
    }
    
    if ($frontendRunning) {
        Write-Status "Frontend:       RUNNING (PID: $frontendPid, http://localhost:8081)" "Green"
    } else {
        Write-Status "Frontend:       STOPPED" "Red"
    }
    
    Write-Status "" "White"
}

function Start-All {
    $BackendExe = Join-Path $BuildDir "backend\drone-backend.exe"
    $SimulatorExe = Join-Path $BuildDir "simulator\drone-simulator.exe"

    if (-not (Test-Path $BackendExe)) {
        Write-Status "[ERROR] Backend executable not found: $BackendExe" "Red"
        Write-Status "Please run 'build.ps1' first to compile the project." "Yellow"
        return
    }

    if (-not (Test-Path $SimulatorExe)) {
        Write-Status "[ERROR] Simulator executable not found: $SimulatorExe" "Red"
        Write-Status "Please run 'build.ps1' first to compile the project." "Yellow"
        return
    }

    Stop-All
    Start-Sleep -Seconds 1

    Write-Status "Starting Backend Server (port 8080)..." "Green"
    $script:BackendProcess = Start-Process -FilePath $BackendExe -WorkingDirectory $BuildDir -PassThru -WindowStyle Hidden
    Write-Status "Backend PID: $($script:BackendProcess.Id)" "Gray"

    Start-Sleep -Seconds 2

    try {
        $Response = Invoke-WebRequest -Uri "http://localhost:8080/" -TimeoutSec 5 -UseBasicParsing
        Write-Status "Backend is ready!" "Green"
    } catch {
        Write-Status "[ERROR] Backend failed to start" "Red"
        return
    }

    Write-Status "Starting Drone Simulator..." "Green"
    $script:SimulatorProcess = Start-Process -FilePath $SimulatorExe -WorkingDirectory $BuildDir -PassThru -WindowStyle Hidden
    Write-Status "Simulator PID: $($script:SimulatorProcess.Id)" "Gray"

    Write-Status "Starting Frontend Server (port 8081)..." "Green"
    $script:FrontendProcess = Start-Process -FilePath "python" -ArgumentList "-m", "http.server", "8081" -WorkingDirectory $FrontendDir -PassThru -WindowStyle Hidden
    Write-Status "Frontend PID: $($script:FrontendProcess.Id)" "Gray"

    Start-Sleep -Seconds 2

    Save-Pids

    Write-Status "" "White"
    Write-Status "========================================" "Cyan"
    Write-Status "All components started successfully!" "Cyan"
    Write-Status "========================================" "Cyan"
    Write-Status "" "White"
    Write-Status "Services:" "White"
    Write-Status "  Backend API:   http://localhost:8080" "Yellow"
    Write-Status "  Frontend Web:  http://localhost:8081" "Yellow"
    Write-Status "" "White"
    Write-Status "Pages:" "White"
    Write-Status "  Drone List:    http://localhost:8081/index.html" "Yellow"
    Write-Status "  Patrol Map:    http://localhost:8081/map.html" "Yellow"
    Write-Status "  Alert Center:  http://localhost:8081/alerts.html" "Yellow"
}

function Show-Menu {
    Write-Status "" "White"
    Write-Status "========================================" "Cyan"
    Write-Status "  Drone Monitoring System" "Cyan"
    Write-Status "========================================" "Cyan"
    Write-Status "" "White"
    Write-Status "[1] Start All Services" "White"
    Write-Status "[2] Stop All Services" "White"
    Write-Status "[3] Check Status" "White"
    Write-Status "[4] Exit" "White"
    Write-Status "" "White"
    Write-Status "Enter your choice (1-4): " "White" -NoNewline
}

do {
    Show-Menu
    $choice = Read-Host
    
    switch ($choice) {
        "1" { Start-All }
        "2" { Stop-All }
        "3" { Get-ServiceStatus }
        "4" { 
            Stop-All
            Write-Status "Goodbye!" "Green"
            break 
        }
        default { Write-Status "Invalid choice, please enter 1-4" "Red" }
    }
} while ($choice -ne "4")