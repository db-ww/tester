# Build Optimization Guide

To make your ESP32 builds as fast as possible, follow these tips:

## 1. Install `ccache` (Highly Recommended)
`ccache` is a compiler cache. It speeds up recompilation by caching previous compilations and detecting when the same compilation is being done again.

### Installation (Windows):
1. Download a pre-compiled binary from [ccache.dev](https://ccache.dev/download.html).
2. Add `ccache.exe` to your Windows PATH.
3. The ESP32 Arduino core will automatically detect and use it if it's in your PATH.

## 2. Leverage Local Libraries
By moving code into the `libraries/SpeedReaderCore` folder, `arduino-cli` can compile those modules into separate object files. 

### Why this is faster:
- If you only change `speed_reader.ino`, the libraries are **not** recompiled.
- If you change one file in the library (e.g., `SR_Session.cpp`), only that file is recompiled.

## 3. Minimize Header Dependencies
We have refactored `globals.h` to use **forward declarations**.
- Avoid adding `#include <GiantLibrary.h>` to `globals.h`.
- Only include headers in the specific `.cpp` files that need them.
- This prevents a single change from triggering a full rebuild of the entire project.

## 4. Use the Persistent Build Path
The `build.bat` script is configured to use `--build-path "%SKETCH_PATH%\build\obj"`.
- **Do not** use the `-Clean` flag unless you are seeing strange errors.
- Keeping the build folder allows `arduino-cli` to reuse existing `.o` files.

## 5. Parallel Compilation
The script uses `-j 0`, which tells `arduino-cli` to use all available CPU cores on your machine.

## 6. Advanced: Antivirus Exclusions (Windows Only)
This is the most common cause of slow builds on Windows. Antivirus software scans every `.o` file created by the compiler.
- **Action**: Add `D:\woodway\apps\Arduino\Tester\speed_reader\build` to your **Windows Defender Exclusion List**.
- This alone can often cut build times by **30% - 50%**.

## 7. Advanced: RAM Disk or SSD
If your project is on a mechanical HDD (common for `D:` drives), moving the compiler's temporary files to an SSD or a RAM Disk will drastically reduce I/O wait times.
- Our `build.bat` script is already optimized to keep your build folder persistent, so once it's on a fast drive, rebuilds are nearly instant.
