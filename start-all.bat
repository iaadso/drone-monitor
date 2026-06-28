@echo off
echo Starting Drone Monitor System...

cd /d "%~dp0"

echo Starting Backend Server...
start /min "Backend" build\backend\drone-backend.exe
timeout /t 3 /nobreak >nul

echo Starting Drone Simulator...
start /min "Simulator" build\simulator\drone-simulator.exe
timeout /t 3 /nobreak >nul

echo Starting Frontend Server...
start /min "Frontend" cmd /k "cd frontend && C:\Python314\python.exe -m http.server 8000"
timeout /t 2 /nobreak >nul

echo System started successfully!
echo Frontend: http://localhost:8000
echo Backend API: http://localhost:8080/api
pause