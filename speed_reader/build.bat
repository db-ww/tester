@echo off
REM Build and Upload Script for speed_reader ESP32 Sketch
REM Batch script to compile and upload the sketch using arduino-cli
REM Also handles SPIFFS data upload

setlocal enabledelayedexpansion

REM Default parameters
set "BOARD=esp32:esp32:esp32:PartitionScheme=huge_app"
set "PORT="
set "BUILD_ONLY=0"
set "UPLOAD_ONLY=0"
set "SKIP_DATA=0"

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
if /i "%~1"=="-UploadOnly" (
    set "UPLOAD_ONLY=1"
    shift
    goto parse_args
)
if /i "%~1"=="-SkipData" (
    set "SKIP_DATA=1"
    shift
    goto parse_args
)
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
shift
goto parse_args

:show_help
echo Usage: build.bat [options]
echo.
echo Options:
echo   -Port COMx        Serial port (default: Auto-detect)
echo   -BuildOnly        Compile only without uploading
echo   -UploadOnly       Upload only without compiling (fast)
echo   -SkipData         Skip uploading SPIFFS data
echo   -h, --help        Show this help message
echo.
echo Examples:
echo   build.bat
echo   build.bat -Port COM5
echo   build.bat -BuildOnly
echo   build.bat -UploadOnly
exit /b 0

:args_done
REM Auto-detect port if not specified and we need to upload
if "%PORT%"=="" (
    if %BUILD_ONLY% equ 0 (
        echo Auto-detecting COM port...
        for /f "tokens=1" %%a in ('arduino-cli board list ^| findstr "COM"') do (
            set "PORT=%%a"
            goto port_found
        )
        echo Error: No COM port found!
        echo Please specify port using -Port flag or connect device.
        exit /b 1
    )
)

:port_found
REM Get script directory
set "SCRIPT_DIR=%~dp0"
REM Fix for when script is run in a way that returns invalid path
if "%SCRIPT_DIR%"=="\\.\" set "SCRIPT_DIR=%CD%\"
set "SKETCH_PATH=%SCRIPT_DIR:~0,-1%"
set "SKETCH_NAME=speed_reader.ino"
set "DATA_DIR=%SKETCH_PATH%\data"

REM Tool Paths (Adjust versions if necessary)
set "ARDUINO_PACKAGES=%LOCALAPPDATA%\Arduino15\packages\esp32"
set "ESPTOOL=%ARDUINO_PACKAGES%\tools\esptool_py\5.1.0\esptool.exe"
set "MKSPIFFS=%ARDUINO_PACKAGES%\tools\mkspiffs\0.2.3\mkspiffs.exe"

REM Partition settings for Huge App
set "SPIFFS_OFFSET=0x310000"
set "SPIFFS_SIZE=0xE0000"

cls
echo ========================================
echo ESP32 Speed Reader Build ^& Upload Tool
echo ========================================
echo.
echo Script directory: %SCRIPT_DIR%
echo Sketch: %SKETCH_NAME%
echo Board: %BOARD%
echo Port: %PORT%
echo Build only: %BUILD_ONLY%
echo Upload only: %UPLOAD_ONLY%
echo Skip Data: %SKIP_DATA%
echo.

REM Check if arduino-cli is available
where arduino-cli >nul 2>&1
if errorlevel 1 (
    echo ERROR: arduino-cli is not installed or not in PATH
    exit /b 1
)

echo Found arduino-cli
echo.

REM Check if upload-only mode
if %UPLOAD_ONLY% equ 1 (
    echo Upload-only mode. Skipping compilation.
    echo.
    goto upload_sketch
)

REM Compile the sketch
echo Compiling sketch...
echo.
call arduino-cli compile --fqbn %BOARD% "%SKETCH_PATH%" --verbose

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

:upload_sketch
REM Upload the sketch
echo Uploading sketch to board...
echo.
call arduino-cli upload -p %PORT% --fqbn %BOARD% "%SKETCH_PATH%" --verbose

if errorlevel 1 (
    echo.
    echo Sketch upload failed!
    exit /b 1
)

echo Sketch upload successful!
echo.

REM Upload SPIFFS Data
if %SKIP_DATA% equ 0 (
    echo Processing SPIFFS data...
    
    if not exist "%DATA_DIR%" (
        echo Warning: data directory not found at %DATA_DIR%
        goto skip_data_upload
    )

    if not exist "%MKSPIFFS%" (
        echo Warning: mkspiffs not found at %MKSPIFFS%
        echo Skipping data upload.
        goto skip_data_upload
    )

    if not exist "%ESPTOOL%" (
        echo Warning: esptool not found at %ESPTOOL%
        echo Skipping data upload.
        goto skip_data_upload
    )

    echo Creating SPIFFS image...
    "%MKSPIFFS%" -c "%DATA_DIR%" -b 4096 -p 256 -s %SPIFFS_SIZE% "%SKETCH_PATH%\spiffs.bin"
    
    if errorlevel 1 (
        echo Failed to create SPIFFS image.
        goto skip_data_upload
    )

    echo Uploading SPIFFS image...
    "%ESPTOOL%" --chip esp32 --port %PORT% --baud 921600 --before default_reset --after hard_reset write_flash -z %SPIFFS_OFFSET% "%SKETCH_PATH%\spiffs.bin"

    if errorlevel 1 (
        echo SPIFFS upload failed.
    ) else (
        echo SPIFFS upload successful!
    )
    
    REM Clean up
    if exist "%SKETCH_PATH%\spiffs.bin" del "%SKETCH_PATH%\spiffs.bin"
)

:skip_data_upload
echo.
echo Opening Serial Monitor...
echo Press Ctrl+C to exit Serial Monitor
echo.

REM Open serial monitor at 115200 baud (ESP32 standard)
call arduino-cli monitor -p %PORT% -c baudrate=115200

exit /b 0
