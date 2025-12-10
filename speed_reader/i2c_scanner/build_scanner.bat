@echo off
REM Build and Upload Script for I2C Scanner ESP32 Sketch

setlocal enabledelayedexpansion

REM Default parameters
set "BOARD=esp32:esp32:esp32"
set "PORT=COM8"
set "BUILD_ONLY=0"

REM Parse command line arguments
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="-Port" (
    set "PORT=%~2"
    shift
    shift
    goto parse_args
)
if /i "%~1"=="-BuildOnly" (
    set "BUILD_ONLY=1"
    shift
    goto parse_args
)
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
shift
goto parse_args

:show_help
echo Usage: build_scanner.bat [options]
echo.
echo Options:
echo   -Port COMx        Serial port (default: COM8)
echo   -BuildOnly        Compile only without uploading
echo   -h, --help        Show this help message
exit /b 0

:args_done
REM Get script directory
set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR%"=="\\.\\" set "SCRIPT_DIR=%CD%\\"
set "SKETCH_PATH=%SCRIPT_DIR:~0,-1%"
set "SKETCH_NAME=i2c_scanner.ino"

cls
echo ========================================
echo ESP32 I2C Scanner Build Tool
echo ========================================
echo.
echo Sketch: %SKETCH_NAME%
echo Board: %BOARD%
echo Port: %PORT%
echo Build only: %BUILD_ONLY%
echo.

REM Check if arduino-cli is available
where arduino-cli >nul 2>&1
if errorlevel 1 (
    echo ERROR: arduino-cli is not installed or not in PATH
    exit /b 1
)

echo Found arduino-cli
echo.

REM Verify ESP32 core is installed
echo Checking ESP32 board support...
arduino-cli core list | findstr "esp32:esp32" >nul 2>&1
if errorlevel 1 (
    echo ESP32 core not found. Installing...
    arduino-cli core update-index
    arduino-cli core install esp32:esp32
    if errorlevel 1 (
        echo ERROR: Failed to install ESP32 core
        exit /b 1
    )
) else (
    echo ESP32 core is installed
)
echo.

REM Compile the sketch
echo Compiling sketch...
echo.
arduino-cli compile --fqbn %BOARD% "%SKETCH_PATH%"

if errorlevel 1 (
    echo.
    echo Compilation failed!
    exit /b 1
)

echo.
echo Compilation successful!
echo.

REM Check if build-only mode
if %BUILD_ONLY% equ 1 (
    echo Build-only mode. Skipping upload.
    exit /b 0
)

REM Upload the sketch
echo Uploading sketch to board on %PORT%...
echo.
arduino-cli upload -p %PORT% --fqbn %BOARD% "%SKETCH_PATH%"

if errorlevel 1 (
    echo.
    echo Sketch upload failed!
    exit /b 1
)

echo.
echo Sketch upload successful!
echo.
echo Opening Serial Monitor at 115200 baud...
echo The scanner will display any I2C devices found.
echo Press Ctrl+C to exit Serial Monitor
echo.

REM Open serial monitor
call arduino-cli monitor -p %PORT% -c baudrate=115200

exit /b 0
