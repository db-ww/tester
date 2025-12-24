#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\hc_sr04\\QUICK_REFERENCE.md"
# HC-SR04 ESP32 Project - Quick Reference

## Project Structure

```
hc_sr04/
├── hc_sr04.ino              # Main sketch (GPIO 5, 18)
├── build_hc_sr04.bat        # Build script for main sketch
├── README.md                # Full documentation
├── TROUBLESHOOTING.md       # Detailed troubleshooting guide
│
├── hc_sr04_alt_pins/        # Alternative pins version
│   ├── hc_sr04_alt_pins.ino # Sketch using GPIO 12, 14
│   └── build_alt_pins.bat   # Build script
│
└── pin_test/                # GPIO pin diagnostic tool
    ├── pin_test.ino         # Pin testing sketch
    └── build_pin_test.bat   # Build script
```

## Quick Commands

### Main Sketch (GPIO 5 TRIG, GPIO 18 ECHO)
```bash
cd hc_sr04
build_hc_sr04.bat -p COM8
```

### Alternative Pins (GPIO 12 TRIG, GPIO 14 ECHO)
```bash
cd hc_sr04_alt_pins
build_alt_pins.bat -p COM8
```

### Pin Test Utility
```bash
cd pin_test
build_pin_test.bat -p COM8
```

## Wiring Diagrams

### Main Configuration
```
HC-SR04    ESP32
────────   ─────────
VCC    →   5V
GND    →   GND
TRIG   →   GPIO 5
ECHO   →   GPIO 18
```

### Alternative Configuration
```
HC-SR04    ESP32
────────   ─────────
VCC    →   5V
GND    →   GND
TRIG   →   GPIO 12
ECHO   →   GPIO 14
```

## Troubleshooting Quick Fixes

### Issue: "Raw pulse: 0 μs" (No echo received)

**Most Common Causes:**
1. ⚡ **Power:** HC-SR04 VCC connected to 3.3V instead of 5V
2. 🔌 **Wiring:** TRIG and ECHO pins swapped
3. 🔧 **Hardware:** Faulty sensor or loose connections
4. 📍 **GPIO:** Pin conflict on ESP32

**Quick Fixes:**
1. Connect VCC to **5V pin** (not 3.3V)
2. Double-check all wire connections
3. Try alternative pins version
4. Test with pin_test utility

### Issue: "Port is busy or doesn't exist"

**Solution:** Close the serial monitor before uploading
- Press `Ctrl+C` in the terminal running the monitor
- Or close the terminal window
- Then run the build script again

### Issue: "Upload failed"

**Solutions:**
1. Hold **BOOT button** on ESP32 while uploading
2. Verify correct COM port: `arduino-cli board list`
3. Install ESP32 support: `arduino-cli core install esp32:esp32`

## Expected Output

### Working Sensor ✅
```
Raw pulse: 2345 μs | Distance: 40.12 cm  |  15.79 inches  ✓ Valid reading
Raw pulse: 2401 μs | Distance: 41.08 cm  |  16.17 inches  ✓ Valid reading
```

### Not Working ❌
```
Raw pulse: 0 μs | ERROR: No echo received (timeout)
  -> Check wiring:
     - TRIG connected to GPIO 5?
     - ECHO connected to GPIO 18?
     - VCC connected to 5V or 3.3V?
     - GND connected?
     - Sensor powered on?
```

## Build Script Options

All build scripts accept the following flags:

- `-p <PORT>` or `-Port <PORT>` - Specify COM port
  - Example: `build_hc_sr04.bat -p COM8`
  - Default: COM8

## Serial Monitor

- **Baud Rate:** 115200 (ESP32 standard)
- **Exit:** Press `Ctrl+C`
- **Manual Start:** `arduino-cli monitor -p COM8 -c baudrate=115200`

## Important Notes

⚠️ **Voltage Warning:** HC-SR04 ECHO pin outputs 5V, but ESP32 GPIO pins are 3.3V tolerant. For long-term use, consider:
- Voltage divider (1kΩ + 2kΩ resistors)
- Level shifter module
- 3.3V compatible sensor (HC-SR04P)

💡 **Power Requirement:** HC-SR04 needs 5V to work reliably. Many sensors will not function at 3.3V.

🔧 **GPIO Pin Selection:** Avoid GPIO 0, 2, 12, 15 for critical functions as they affect boot mode.

## Testing Sequence

1. **Start with main sketch** → If works, you're done! ✅
2. **Try alternative pins** → Tests if GPIO 18 has issues
3. **Run pin test** → Verifies GPIO pins are functional
4. **Check hardware** → Inspect wiring and sensor

## Need Help?

See `TROUBLESHOOTING.md` for detailed diagnostic steps and solutions.
