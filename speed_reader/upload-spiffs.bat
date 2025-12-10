@echo off
REM Upload SPIFFS data to ESP32
REM This uploads the contents of the data/ directory to the ESP32's filesystem

setlocal enabledelayedexpansion

set "BOARD=esp32:esp32:esp32"
set "PORT=COM3"

REM Parse arguments
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="-Port" (
    set "PORT=%~2"
    shift
    shift
    goto parse_args
)
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
shift
goto parse_args

:show_help
echo Usage: upload-spiffs.bat [options]
echo.
echo Options:
echo   -Port COMx       Serial port (default: COM3)
echo   -h, --help       Show this help message
echo.
echo This script uploads the data/ directory to ESP32 SPIFFS
echo.
exit /b 0

:args_done
set "SCRIPT_DIR=%~dp0"

echo ========================================
echo ESP32 SPIFFS Upload Tool
echo ========================================
echo.
echo Script directory: %SCRIPT_DIR%
echo Board: %BOARD%
echo Port: %PORT%
echo.

REM Check if arduino-cli is available
where arduino-cli >nul 2>&1
if errorlevel 1 (
    echo ERROR: arduino-cli is not installed or not in PATH
    echo Install from: https://arduino.github.io/arduino-cli/
    echo.
    echo For easier SPIFFS upload, use Arduino IDE:
    echo   Tools ^> ESP32 Sketch Data Upload
    echo   (requires ESP32 Filesystem Uploader plugin)
    exit /b 1
)

echo Found arduino-cli
echo.

REM Note: arduino-cli does not directly support SPIFFS upload
REM This is a limitation. Use Arduino IDE instead or the utility sketch.
echo NOTE: arduino-cli does not natively support SPIFFS upload.
echo.
echo Instead, you have two options:
echo.
echo Option 1: Use Arduino IDE (Recommended)
echo   - Install: ESP32 Filesystem Uploader plugin
echo   - Tools ^> ESP32 Sketch Data Upload
echo.
echo Option 2: Use the SPIFFS config uploader sketch
echo   - Open spiffs_config_uploader.ino
echo   - Edit the WiFi credentials
echo   - Compile and upload to your ESP32
echo.
echo Option 3: Use platformio (if installed)
echo   - Run: pio run --target uploadfs -e esp32-lolin_v3
echo.
exit /b 0
