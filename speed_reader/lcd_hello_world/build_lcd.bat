@echo off
REM Build and Upload Script for LCD Hello World ESP32 Sketch
REM Installs required libraries and compiles/uploads the sketch

setlocal enabledelayedexpansion

REM Default parameters
set "BOARD=esp32:esp32:esp32"
set "PORT=COM7"
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
echo Usage: build_lcd.bat [options]
echo.
echo Options:
echo   -Port COMx        Serial port (default: COM7)
echo   -BuildOnly        Compile only without uploading
echo   -h, --help        Show this help message
echo.
echo Examples:
echo   build_lcd.bat
echo   build_lcd.bat -Port COM5
echo   build_lcd.bat -BuildOnly
exit /b 0

:args_done
REM Get script directory - use pushd/popd for reliable absolute path
pushd "%~dp0" 2>nul || pushd .
set "SKETCH_PATH=%CD%"
popd
set "SKETCH_NAME=lcd_hello_world.ino"

cls
echo ========================================
echo ESP32 LCD Hello World Build Tool
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
    echo Please install arduino-cli from: https://arduino.github.io/arduino-cli/
    exit /b 1
)

echo Found arduino-cli
echo.

REM Install required libraries
echo Installing LiquidCrystal library for parallel LCD...
arduino-cli lib install "LiquidCrystal"
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
    echo.
    echo Common issues:
    echo - Make sure the LiquidCrystal_I2C library is installed
    echo - Check that the ESP32 board support is installed
    echo.
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
    echo.
    echo Common issues:
    echo - Check that the correct COM port is specified
    echo - Make sure the ESP32 is connected
    echo - Try pressing the BOOT button during upload
    echo.
    exit /b 1
)

echo.
echo Sketch upload successful!
echo.
echo Your LCD should now display "Hello, World!"
echo.
echo Wiring reminder for ESP32:
echo   LCD GND  -^> ESP32 GND
echo   LCD VCC  -^> ESP32 5V or 3.3V
echo   LCD SDA  -^> ESP32 GPIO 21
echo   LCD SCL  -^> ESP32 GPIO 22
echo.

arduino-cli monitor --config 115200 -p %PORT%

pause
exit /b 0
