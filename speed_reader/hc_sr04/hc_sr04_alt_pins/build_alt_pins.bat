@echo off
REM Build script for HC-SR04 with alternative pins (ESP32)
REM Uses GPIO 12 (TRIG) and GPIO 14 (ECHO)

echo ====================================
echo HC-SR04 Alternative Pins Build
echo Target: ESP32
echo TRIG: GPIO 12, ECHO: GPIO 14
echo ====================================
echo.
REM Set variables
set SKETCH=hc_sr04_alt_pins.ino
set BOARD=esp32:esp32:esp32
set "PORT=COM8"

REM Parse command line arguments
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="-Port" (
    set "PORT=%~2"
    shift
    shift
    goto parse_args
)
if /i "%~1"=="-p" (
    set "PORT=%~2"
    shift
    shift
    goto parse_args
)

:args_done

echo Compiling %SKETCH%...
echo.

REM Compile the sketch
arduino-cli compile --fqbn %BOARD% %SKETCH%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo.
echo Uploading to ESP32 on %PORT%...
echo.

REM Upload to ESP32
arduino-cli upload -p %PORT% --fqbn %BOARD% %SKETCH%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Upload failed!
    echo Check if the correct port is selected.
    pause
    exit /b 1
)

echo.
echo Upload successful!
echo.
echo Opening Serial Monitor...
echo.

REM Open serial monitor at 115200 baud (ESP32 standard)
arduino-cli monitor -p %PORT% -c baudrate=115200

pause
