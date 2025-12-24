#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\hc_sr04\\README.md"
# HC-SR04 Ultrasonic Distance Sensor - ESP32

This sketch demonstrates how to use the HC-SR04 ultrasonic distance sensor with ESP32.

## Hardware Requirements

- ESP32 Development Board
- HC-SR04 Ultrasonic Distance Sensor
- Jumper wires
- (Optional) Voltage divider or level shifter for ECHO pin

## Wiring Diagram

| HC-SR04 Pin | ESP32 Pin   |
|-------------|-------------|
| VCC         | 5V or 3.3V  |
| GND         | GND         |
| TRIG        | GPIO 5      |
| ECHO        | GPIO 18     |

**⚠️ Important Note:** The HC-SR04 ECHO pin outputs 5V, but ESP32 GPIO pins are only 3.3V tolerant. Consider using:
- A voltage divider (two resistors: 1kΩ and 2kΩ)
- A level shifter module
- A 3.3V compatible HC-SR04 variant

## How It Works

The HC-SR04 sensor measures distance using ultrasonic sound waves:

1. The TRIG pin sends out an ultrasonic pulse
2. The pulse bounces off an object and returns
3. The ECHO pin receives the reflected pulse
4. The time between sending and receiving is measured
5. Distance is calculated using: `Distance = (Time × Speed of Sound) / 2`

## Features

- Displays distance in both centimeters and inches
- Updates readings every 500ms
- Detects out-of-range measurements (valid range: 2-400 cm)
- Serial output at 115200 baud (ESP32 standard)

## Usage

### Build and Upload

Run the build script:
```batch
build_hc_sr04.bat
```

This will:
1. Compile the sketch for ESP32
2. Upload to ESP32 on COM4
3. Open the Serial Monitor automatically at 115200 baud

### Manual Commands

Compile only:
```batch
arduino-cli compile --fqbn esp32:esp32:esp32 hc_sr04.ino
```

Upload:
```batch
arduino-cli upload -p COM4 --fqbn esp32:esp32:esp32 hc_sr04.ino
```

Monitor serial output:
```batch
arduino-cli monitor -p COM4 -c baudrate=115200
```

## Expected Output

```
HC-SR04 Ultrasonic Distance Sensor
===================================

Distance: 15.23 cm  |  5.99 inches
Distance: 15.45 cm  |  6.08 inches
Distance: 20.12 cm  |  7.92 inches
...
```

## Troubleshooting

- **No readings**: Check wiring connections, especially ECHO pin voltage protection
- **Erratic readings**: Ensure sensor is pointed at a flat surface
- **Out of range**: Object is too close (<2cm) or too far (>400cm)
- **Upload failed**: 
  - Verify correct COM port in build script
  - Hold BOOT button on ESP32 while uploading
  - Ensure ESP32 board support is installed: `arduino-cli core install esp32:esp32`
- **Damaged ESP32**: If not using voltage protection on ECHO pin, the 5V signal may damage the GPIO pin over time

## Technical Specifications

- Operating Voltage: 5V DC
- Operating Current: 15mA
- Measuring Range: 2cm - 400cm
- Accuracy: ±3mm
- Measuring Angle: 15°
- Trigger Input Signal: 10µs TTL pulse
- Echo Output Signal: TTL pulse proportional to distance
