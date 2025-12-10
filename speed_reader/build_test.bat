@echo off
REM Build and Upload Script for Hall Sensor Test
setlocal

set "BOARD=esp32:esp32:esp32"
set "PORT=COM8"
set "SKETCH=hall_sensor_test.ino"

REM Parse arguments
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="-Port" (
    set "PORT=%~2"
    shift
    shift
    goto parse_args
)
shift
goto parse_args

:args_done
set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR%"=="\\.\" set "SCRIPT_DIR=%CD%\"
set "SKETCH_PATH=%SCRIPT_DIR%hall_sensor_test\%SKETCH%"

echo Compiling %SKETCH%...
arduino-cli compile --fqbn %BOARD% "%SKETCH_PATH%" --verbose
if errorlevel 1 (
    echo Compilation failed
    exit /b 1
)

echo Uploading to %PORT%...
arduino-cli upload -p %PORT% --fqbn %BOARD% "%SKETCH_PATH%" --verbose
if errorlevel 1 (
    echo Upload failed
    exit /b 1
)

echo Opening Serial Monitor...
arduino-cli monitor -p %PORT% --config 115200
