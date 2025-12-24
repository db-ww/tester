#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\lcd_hello_world\\README.md"
# ESP32 I2C LCD Hello World

This project displays "Hello, World!" on a 16x2 I2C LCD screen using an ESP32.

## Hardware Requirements

- ESP32 Development Board
- 16x2 I2C LCD Display (with I2C backpack)
- Jumper wires

## Wiring Diagram

Connect the LCD to the ESP32 as follows:

| LCD Pin | ESP32 Pin | Description |
|---------|-----------|-------------|
| GND     | GND       | Ground      |
| VCC     | 5V or 3.3V| Power (check your LCD specs) |
| SDA     | GPIO 21   | I2C Data    |
| SCL     | GPIO 22   | I2C Clock   |

## Software Requirements

- [arduino-cli](https://arduino.github.io/arduino-cli/) installed and in PATH
- ESP32 board support (will be installed automatically by the build script)
- LiquidCrystal_I2C library (will be installed automatically by the build script)

## Quick Start

### Option 1: Find LCD Address First (Recommended)

1. Wire your LCD to the ESP32 as shown above
2. Run the I2C scanner:
   ```batch
   cd i2c_scanner
   build_scanner.bat
   ```
3. Note the I2C address shown in the Serial Monitor (usually 0x27 or 0x3F)
4. If the address is different from 0x27, edit `lcd_hello_world.ino` line 20:
   ```cpp
   LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  // Change 0x27 to your address
   ```

### Option 2: Upload Hello World Directly

If you know your LCD address is 0x27 (most common):

1. Wire your LCD to the ESP32
2. Run the build script:
   ```batch
   cd lcd_hello_world
   build_lcd.bat
   ```
3. The LCD should display "Hello, World!"

## Build Scripts

### `build_lcd.bat`
Builds and uploads the Hello World sketch to the ESP32.

**Usage:**
```batch
build_lcd.bat                    # Build and upload to COM8
build_lcd.bat -Port COM5         # Use different port
build_lcd.bat -BuildOnly         # Compile only, don't upload
build_lcd.bat -h                 # Show help
```

**Features:**
- Automatically installs LiquidCrystal_I2C library
- Verifies ESP32 core is installed
- Compiles and uploads the sketch
- Shows wiring reminder after upload

### `build_scanner.bat`
Builds and uploads the I2C scanner sketch.

**Usage:**
```batch
build_scanner.bat                # Build, upload, and open serial monitor
build_scanner.bat -Port COM5     # Use different port
build_scanner.bat -BuildOnly     # Compile only
```

**Features:**
- Automatically opens Serial Monitor at 115200 baud
- Scans for I2C devices every 5 seconds
- Displays found device addresses

## Troubleshooting

### LCD shows nothing
1. Check wiring connections
2. Verify LCD has power (backlight should be on)
3. Run I2C scanner to verify LCD is detected
4. Check if LCD address is correct (0x27 or 0x3F)
5. Try adjusting the contrast potentiometer on the I2C backpack

### Upload fails
1. Verify correct COM port (check Device Manager)
2. Press and hold BOOT button during upload
3. Make sure no other program is using the serial port
4. Try a different USB cable

### I2C Scanner finds no devices
1. Double-check wiring (especially SDA and SCL)
2. Verify LCD has power
3. Try swapping SDA and SCL pins
4. Check if I2C backpack is properly soldered to LCD

### Compilation errors
1. Make sure arduino-cli is installed and in PATH
2. Let the build script install the required libraries
3. Check that ESP32 board support is installed

## Code Structure

### lcd_hello_world.ino
- Initializes I2C on GPIO 21 (SDA) and GPIO 22 (SCL)
- Configures LCD for 16 columns x 2 rows
- Displays "Hello, World!" on the first row
- Static display (no loop updates)

### i2c_scanner.ino
- Scans I2C bus addresses 1-126
- Reports found devices via Serial Monitor
- Runs scan every 5 seconds
- Useful for debugging I2C connections

## Customization

### Change the message
Edit `lcd_hello_world.ino`:
```cpp
lcd.print("Hello, World!");  // Change this text
```

### Display on second row
```cpp
lcd.setCursor(0, 1);  // Column 0, Row 1 (second row)
lcd.print("Your text");
```

### Add scrolling or animations
See the [Random Nerd Tutorials guide](https://randomnerdtutorials.com/esp32-esp8266-i2c-lcd-arduino-ide/) for more examples.

## Resources

- [Random Nerd Tutorials - ESP32 I2C LCD Guide](https://randomnerdtutorials.com/esp32-esp8266-i2c-lcd-arduino-ide/)
- [LiquidCrystal_I2C Library Documentation](https://github.com/marcoschwartz/LiquidCrystal_I2C)
- [ESP32 I2C Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)

## License

This code is based on examples from Random Nerd Tutorials and is provided as-is for educational purposes.
