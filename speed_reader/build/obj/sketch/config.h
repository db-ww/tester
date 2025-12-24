#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\config.h"
#ifndef CONFIG_H
#define CONFIG_H

// Feature toggles: set to 1 to enable, 0 to disable to reduce binary size.
#define ENABLE_HTTP 1
#define ENABLE_BT 1

// ===== Pin Definitions =====
const int LED_PIN = 2;
const int D4_DIGITAL = 4;   // rotation pulse (Changed to 4 for internal pull-up support)
const int D5_ANALOG = 35;   // ADC1 pin
const int I2C_SDA = 26;     // SDA pin (Avoid 21 - used by LCD)
const int I2C_SCL = 27;     // SCL pin (Avoid 22 - used by LCD)
const int ACCEL_INT = 32;   // Optional: Accelerometer Interrupt (connect INT here)

// LCD Pins: RS, E, D4, D5, D6, D7
const int LCD_RS = 23;
const int LCD_E = 22;
const int LCD_D4 = 5;
const int LCD_D5 = 18;
const int LCD_D6 = 19;
const int LCD_D7 = 21;
const int LCD_CONTRAST = 25;  // V0 pin - connect LCD contrast to this GPIO

// ===== LCD Settings =====
const int LCD_CONTRAST_VALUE = 100;  // 0-255: Increased to 120 to fix solid blocks (too dark)

// ===== Timing Constants =====
const unsigned long readIntervalMs = 200;
const unsigned long speedTimeoutMs = 2000UL;

// ===== Physical Constants =====
const float wheelDiameterIn = 3.5f;

#endif // CONFIG_H
