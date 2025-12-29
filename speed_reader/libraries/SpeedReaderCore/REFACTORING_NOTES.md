# SpeedReaderCore Library Refactoring

## Summary

The SpeedReaderCore local library has been created to speed up Arduino builds by separating implementation code (`.cpp`) from declarations (`.h`). This allows Arduino to compile the library files once and cache them, only recompiling when changed.

## Completed Refactorings

### ✅ Modules Moved to Library

1. **SR_LCDDisplay** - LCD display functions
   - `updateLCD()`, `showReady()`, `showJob()`, `showSpeed()`
   
2. **SR_Session** - Session management  
   - `resetSession()`, `startSession()`, `endSession()`
   
3. **SR_SpeedSensor** - Speed calculations and ISR
   - `onRotation()` interrupt handler
   - `getCurrentSpeed()` calculations
   
4. **SR_WiFiLoader** - Configuration loading/saving
   - `loadWiFiConfig()`, `saveConfig()`
   - JSON parsing helpers: `getJsonValue()`, `encryptDecrypt()`, `decryptHex()`

## Remaining Files to Refactor

These files should be refactored in a similar pattern for maximum build speed:

1. **accelerometer.h** → **SR_Accelerometer.h/.cpp**
   - Complex IMU fusion code (292 lines)
   - Functions: `initAccelerometer()`, `updateAngle()`, `calibrateAccelerometer()`
   
2. **http_handlers.h** → **SR_HTTPHandlers.h/.cpp**
   - HTTPS endpoint handlers (229 lines)
   - Functions: `handleRoot()`, `handleStart()`, `handleReadings()`, `handleConfig()`
   
3. **tasks.h** → **SR_Tasks.h/.cpp**
   - FreeRTOS task implementations (147 lines)
   - Functions: `sensorTask()`, `displayTask()`
   
4. **startup_check.h** → **SR_StartupCheck.h/.cpp**
   - Diagnostic routines (78 lines)
   - Function: `runStartupDiagnostics()`

## Files Kept as Headers

These remain header-only due to their simple nature:

- `config.h` - Compile-time constants only
- `globals.h` - Global variable declarations (extern)
- `wifi_config.h` - Simple config structure

## Library Structure

```
lib/
└── SpeedReaderCore/
    ├── library.properties
    └── src/
        ├── SR_LCDDisplay.h/.cpp
        ├── SR_Session.h/.cpp
        ├── SR_SpeedSensor.h/.cpp
        ├── SR_WiFiLoader.h/.cpp
        ├── SR_Accelerometer.h/.cpp (TODO)
        ├── SR_HTTPHandlers.h/.cpp (TODO)
        ├── SR_Tasks.h/.cpp (TODO)
        └── SR_StartupCheck.h/.cpp (TODO)
```

## Usage in Main Sketch

Replace old header includes:
```cpp
#include "lcd_display.h"
#include "session.h"
#include "speed_sensor.h"
#include "wifi_loader.h"
```

With new library includes:
```cpp
#include <SR_LCDDisplay.h>
#include <SR_Session.h>
#include <SR_SpeedSensor.h>
#include <SR_WiFiLoader.h>
```

## Build Time Improvements

**Before**: All header-only files recompile on every build (~30-60s)
**After**: Library `.cpp` files compile once, only recompile when modified (~5-15s for sketch-only changes)

**Expected savings**: 50-75% reduction in build time for incremental changes
