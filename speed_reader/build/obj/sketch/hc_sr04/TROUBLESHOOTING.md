#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\hc_sr04\\TROUBLESHOOTING.md"
# HC-SR04 Troubleshooting Guide

## Current Issue: No Echo Signal (Raw pulse: 0 μs)

This means the ESP32 is sending trigger pulses but not receiving any echo response from the HC-SR04 sensor.

## Diagnostic Steps

### Step 1: Verify Power Supply ⚡

The HC-SR04 requires **5V** to operate reliably. Many sensors will not work at 3.3V.

**Action:**
1. Connect HC-SR04 VCC to ESP32 **5V pin** (not 3.3V)
2. Verify the 5V pin is actually providing 5V with a multimeter
3. Check that the ESP32 is powered via USB (provides 5V)

### Step 2: Check Wiring 🔌

**Current Pin Configuration (Main Sketch):**
```
HC-SR04 VCC  → ESP32 5V
HC-SR04 GND  → ESP32 GND
HC-SR04 TRIG → ESP32 GPIO 5
HC-SR04 ECHO → ESP32 GPIO 18
```

**Alternative Pin Configuration:**
```
HC-SR04 VCC  → ESP32 5V
HC-SR04 GND  → ESP32 GND
HC-SR04 TRIG → ESP32 GPIO 12
HC-SR04 ECHO → ESP32 GPIO 14
```

**Common Wiring Mistakes:**
- ❌ TRIG and ECHO swapped
- ❌ Using 3.3V instead of 5V
- ❌ Loose connections
- ❌ Wrong GPIO pins

### Step 3: Test Alternative Pins 🔄

Some ESP32 boards have issues with certain GPIO pins. Try the alternative configuration:

**Run:**
```bash
cd hc_sr04_alt_pins
build_alt_pins.bat -p COM8
```

This uses GPIO 12 (TRIG) and GPIO 14 (ECHO) instead.

### Step 4: Run Pin Test 🧪

Test if the GPIO pins themselves are working:

**Run:**
```bash
cd pin_test
build_pin_test.bat -p COM8
```

This will:
1. Toggle the TRIG pin (you can verify with an LED or multimeter)
2. Read the ECHO pin state
3. Allow you to manually test by connecting ECHO to 3.3V or GND

### Step 5: Test Sensor Hardware 🔧

**Visual Inspection:**
- Check for physical damage on the HC-SR04
- Verify the ultrasonic transducers (silver cylinders) are intact
- Look for cold solder joints

**Multimeter Test:**
1. Measure voltage at HC-SR04 VCC pin (should be ~5V)
2. Check continuity of all wires
3. Verify GND connection

### Step 6: Try Different Sensor 🔄

If available, test with a different HC-SR04 module to rule out a faulty sensor.

## Known Issues with ESP32 + HC-SR04

### Issue 1: Voltage Level Mismatch
- **Problem:** HC-SR04 ECHO outputs 5V, ESP32 GPIO is 3.3V tolerant
- **Risk:** May damage ESP32 over time
- **Solution:** Use voltage divider or level shifter on ECHO pin
  - Simple voltage divider: 1kΩ resistor from ECHO to GPIO, 2kΩ resistor from GPIO to GND

### Issue 2: GPIO Pin Conflicts
- **Problem:** Some GPIO pins are used by ESP32 for boot/flash
- **Affected Pins:** GPIO 0, 2, 12, 15 (boot mode pins)
- **Solution:** Use safe GPIO pins like 5, 13, 14, 16, 17, 18, 19, 21, 22, 23

### Issue 3: Power Supply
- **Problem:** HC-SR04 draws current spikes during operation
- **Solution:** Ensure stable 5V supply, consider adding 100μF capacitor near sensor

## Quick Test Checklist ✓

- [ ] HC-SR04 VCC connected to 5V (not 3.3V)
- [ ] GND connected properly
- [ ] TRIG connected to correct GPIO pin
- [ ] ECHO connected to correct GPIO pin
- [ ] All connections are secure (no loose wires)
- [ ] ESP32 powered via USB
- [ ] Correct COM port selected
- [ ] No obstacles blocking sensor (test with hand 10-20cm away)
- [ ] Sensor is not pointing at soft/angled surfaces

## Testing Sequence

1. **Test with main sketch** (GPIO 5, 18)
   ```bash
   cd hc_sr04
   build_hc_sr04.bat -p COM8
   ```

2. **If fails, test alternative pins** (GPIO 12, 14)
   ```bash
   cd hc_sr04_alt_pins
   build_alt_pins.bat -p COM8
   ```

3. **If still fails, run pin test**
   ```bash
   cd pin_test
   build_pin_test.bat -p COM8
   ```

4. **If pin test passes, check sensor hardware**

## Expected Output (Working Sensor)

```
Raw pulse: 2345 μs | Distance: 40.12 cm  |  15.79 inches  ✓ Valid reading
Raw pulse: 2401 μs | Distance: 41.08 cm  |  16.17 inches  ✓ Valid reading
```

## Expected Output (Faulty Sensor)

```
Raw pulse: 0 μs | ERROR: No echo received (timeout)
  -> Check wiring:
     - TRIG connected to GPIO X?
     - ECHO connected to GPIO Y?
     ...
```

## Additional Resources

- HC-SR04 Datasheet: [Link to datasheet]
- ESP32 GPIO Pin Reference: GPIO 0-39 available, but some have restrictions
- Voltage Divider Calculator: R1=1kΩ, R2=2kΩ for 5V→3.3V conversion

## Still Not Working?

If you've tried all the above steps and the sensor still doesn't work:

1. **Verify sensor with Arduino Uno** (5V logic) to confirm it's not faulty
2. **Try a different ESP32 board** to rule out board issues
3. **Consider using a 3.3V compatible ultrasonic sensor** (HC-SR04P or JSN-SR04T)
