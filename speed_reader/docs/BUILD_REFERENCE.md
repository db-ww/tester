# Build Script Quick Reference

## Usage
```batch
build.bat [options]
```

## Options

### `-Port COMx`
Specify the serial port (default: COM8)
```batch
build.bat -Port COM5
```

### `-BuildOnly`
Compile only without uploading
```batch
build.bat -BuildOnly
```

### `-UploadOnly` ⚡ **NEW - FAST!**
Upload only without compiling (saves time!)
```batch
build.bat -UploadOnly
```
**Use this when:**
- You've already compiled once
- You just need to re-upload the same binary
- You want to flash multiple boards with the same code
- **Saves 30-60 seconds per upload!**

### `-SkipData`
Skip uploading SPIFFS data
```batch
build.bat -SkipData
```

### `-h` or `--help`
Show help message

## Common Workflows

### First Time / After Code Changes
```batch
build.bat
```
Compiles, uploads sketch, uploads SPIFFS data, and opens serial monitor.

### Quick Re-upload (FAST) ⚡
```batch
build.bat -UploadOnly
```
Skips compilation, just uploads the previously compiled binary.

### Test Compilation Only
```batch
build.bat -BuildOnly
```
Compiles to check for errors, doesn't upload.

### Upload to Different Port
```batch
build.bat -Port COM5 -UploadOnly
```

### Skip SPIFFS Upload
```batch
build.bat -SkipData
```

## Time Savings

| Command | Compile | Upload | SPIFFS | Monitor | Total Time |
|---------|---------|--------|--------|---------|------------|
| `build.bat` | ✅ 30-60s | ✅ 10s | ✅ 5s | ✅ | ~45-75s |
| `build.bat -UploadOnly` | ❌ | ✅ 10s | ✅ 5s | ✅ | ~15s ⚡ |
| `build.bat -BuildOnly` | ✅ 30-60s | ❌ | ❌ | ❌ | ~30-60s |

**💡 Tip:** Use `-UploadOnly` for rapid testing when you haven't changed the code!

## Serial Monitor

After upload, the script automatically opens the serial monitor at **115200 baud**.

Press `Ctrl+C` to exit the serial monitor.

## Examples

```batch
# Standard build and upload
build.bat

# Fast re-upload to same board
build.bat -UploadOnly

# Upload to different port
build.bat -Port COM3 -UploadOnly

# Just compile to check for errors
build.bat -BuildOnly

# Full build without SPIFFS data
build.bat -SkipData
```
