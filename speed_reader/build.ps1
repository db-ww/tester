# Build and Upload Script for speed_reader ESP32 Sketch
# PowerShell script to compile and upload the sketch using arduino-cli

param(
    [string]$Board = "esp32:esp32:esp32",
    [string]$Port = "COM3",
    [switch]$BuildOnly = $false
)

$ErrorActionPreference = "Stop"

# Paths
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sketchPath = $scriptDir
$sketchName = "speed_reader.ino"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP32 Speed Reader Build & Upload Tool" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Script directory: $scriptDir" -ForegroundColor Gray
Write-Host "Sketch: $sketchName" -ForegroundColor Gray
Write-Host "Board: $Board" -ForegroundColor Gray
Write-Host "Port: $Port" -ForegroundColor Gray
Write-Host "Build only: $BuildOnly" -ForegroundColor Gray
Write-Host ""

# Check if arduino-cli is available
try {
    $cliVersion = & arduino-cli version 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "arduino-cli not found"
    }
    Write-Host "Found arduino-cli" -ForegroundColor Green
} catch {
    Write-Host "ERROR: arduino-cli is not installed or not in PATH" -ForegroundColor Red
    Write-Host "Install from: https://arduino.github.io/arduino-cli/" -ForegroundColor Yellow
    exit 1
}

# Compile the sketch
Write-Host ""
Write-Host "Compiling sketch..." -ForegroundColor Cyan
$compileCmd = "arduino-cli compile --fqbn $Board `"$sketchPath`" --verbose"
Invoke-Expression $compileCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Compilation successful!" -ForegroundColor Green

if ($BuildOnly) {
    Write-Host ""
    Write-Host "Build-only mode. Skipping upload." -ForegroundColor Cyan
    exit 0
}

# Upload the sketch
Write-Host ""
Write-Host "Uploading to board..." -ForegroundColor Cyan
$uploadCmd = "arduino-cli upload -p $Port --fqbn $Board `"$sketchPath`" --verbose"
Invoke-Expression $uploadCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host "Upload failed!" -ForegroundColor Red
    Write-Host "Check that:" -ForegroundColor Yellow
    Write-Host "  - The board is connected to port $Port" -ForegroundColor Yellow
    Write-Host "  - The correct port is specified (use -Port COMx parameter)" -ForegroundColor Yellow
    Write-Host "  - The board is not in use by another process" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "Upload successful!" -ForegroundColor Green
Write-Host ""
Write-Host "Opening Serial Monitor..." -ForegroundColor Cyan
Write-Host "Press Ctrl+C to exit Serial Monitor" -ForegroundColor Gray
Write-Host ""

# Optional: Open serial monitor with arduino-cli
$monitorCmd = "arduino-cli monitor -p $Port"
Invoke-Expression $monitorCmd
