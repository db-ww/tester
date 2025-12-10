# Quick build and upload helper script
# Usage examples:
#   .\build-and-upload.ps1              # Build and upload to default COM3
#   .\build-and-upload.ps1 -Port COM5   # Upload to specific port
#   .\build-and-upload.ps1 -BuildOnly   # Compile only (don't upload)

param(
    [string]$Port = "COM3",
    [switch]$BuildOnly = $false
)

$board = "esp32:esp32:esp32"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Show usage
if ($args -contains "-h" -or $args -contains "--help") {
    Write-Host "Usage: .\build-and-upload.ps1 [options]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Port <COM#>       Serial port (default: COM3)"
    Write-Host "  -BuildOnly         Compile only without uploading"
    Write-Host "  -h, --help         Show this help message"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build-and-upload.ps1"
    Write-Host "  .\build-and-upload.ps1 -Port COM5"
    Write-Host "  .\build-and-upload.ps1 -BuildOnly"
    exit 0
}

# Call the main build script
& "$scriptDir\build.ps1" -Board $board -Port $Port -BuildOnly:$BuildOnly
