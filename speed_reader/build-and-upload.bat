@echo off
REM Quick build and upload helper script
REM Usage examples:
REM   build-and-upload.bat
REM   build-and-upload.bat -Port COM5
REM   build-and-upload.bat -BuildOnly

setlocal enabledelayedexpansion

set "BOARD=esp32:esp32:esp32"
set "PORT=COM3"
set "BUILD_ONLY=0"

REM Parse arguments
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
echo Usage: build-and-upload.bat [options]
echo.
echo Options:
echo   -Port COMx       Serial port (default: COM3)
echo   -BuildOnly       Compile only without uploading
echo   -h, --help       Show this help message
echo.
echo Examples:
echo   build-and-upload.bat
echo   build-and-upload.bat -Port COM5
echo   build-and-upload.bat -BuildOnly
exit /b 0

:args_done
set "SCRIPT_DIR=%~dp0"
call "%SCRIPT_DIR%build.bat" -Port %PORT%
if %BUILD_ONLY% equ 1 (
    call "%SCRIPT_DIR%build.bat" -Port %PORT% -BuildOnly
) else (
    call "%SCRIPT_DIR%build.bat" -Port %PORT%
)
exit /b %errorlevel%
