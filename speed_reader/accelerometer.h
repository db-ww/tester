#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Wire.h>
#include <math.h>
#include "globals.h"

#define ACCEL_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_CONFIG 0x1A
#define MPU6050_ACCEL_XOUT_H 0x3B

void writeAccelRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ACCEL_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void initAccelerometer() {
  Serial.println("Initializing GY-521 (MPU6050)...");
  
  // Initialize I2C with defined pins
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400kHz fast mode for smoother reads on GY-521
  
  // MPU6050 Init Sequence
  // 1. Wake up and set Clock Source to PLL with X Axis Gyro reference (More stable)
  // Register 0x6B (PWR_MGMT_1) = 0x01
  writeAccelRegister(MPU6050_PWR_MGMT_1, 0x01);
  delay(100); // Wait for PLL to settle

  // 2. Set DLPF (Digital Low Pass Filter) to Mode 5 (10Hz bandwidth)
  // This reduces noise and improves effective resolution for static/slow measurements
  writeAccelRegister(MPU6050_CONFIG, 0x05);
  delay(10);
  
  Serial.println("GY-521 init done (PLL + DLPF Enabled).");
}

// Low-pass filter coefficient (0.0 - 1.0)
// Higher = more smoothing, slower response
const float ALPHA = 0.02f;
float filtered_ax = 0.0f;
float filtered_ay = 0.0f;
float filtered_az = 0.0f;
// 3D Base Vector
float base_x = 0.0f;
float base_y = 0.0f;
float base_z = 1.0f; // Default to Z-up

// Debug: Raw values
int16_t debug_raw_x = 0;
int16_t debug_raw_y = 0;
int16_t debug_raw_z = 0;

void updateAngle() {
  Wire.beginTransmission(ACCEL_ADDR);
  Wire.write(MPU6050_ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) {
    // I2C Error
    return;
  }
  
  Wire.requestFrom((uint8_t)ACCEL_ADDR, (uint8_t)6);
  if (Wire.available() >= 6) {
    int16_t x = (Wire.read() << 8) | Wire.read();
    int16_t y = (Wire.read() << 8) | Wire.read();
    int16_t z = (Wire.read() << 8) | Wire.read();
    
    debug_raw_x = x;
    debug_raw_y = y;
    debug_raw_z = z;
    
    // Scale to g (assuming default +/- 2g range, 16384 LSB/g)
    float ax = x / 16384.0f;
    float ay = y / 16384.0f;
    float az = z / 16384.0f;

    // Initialize filtered values on first run
    if (filtered_az == 0.0f && filtered_ay == 0.0f && filtered_ax == 0.0f) {
        filtered_ax = ax;
        filtered_ay = ay;
        filtered_az = az;
    }
    
    // Apply low-pass filter
    filtered_ax = (ALPHA * ax) + ((1.0f - ALPHA) * filtered_ax);
    filtered_ay = (ALPHA * ay) + ((1.0f - ALPHA) * filtered_ay);
    filtered_az = (ALPHA * az) + ((1.0f - ALPHA) * filtered_az);
    
    // Calculate 3D Angle deviation from Base Vector using Dot Product
    // Cos(theta) = (A . B) / (|A| * |B|)
    
    // 1. Calculate Dot Product
    float dot = (filtered_ax * base_x) + (filtered_ay * base_y) + (filtered_az * base_z);
    
    // 2. Calculate Magnitudes
    float mag_curr = sqrt(filtered_ax*filtered_ax + filtered_ay*filtered_ay + filtered_az*filtered_az);
    float mag_base = sqrt(base_x*base_x + base_y*base_y + base_z*base_z);
    
    // 3. Calculate Cosine
    // Protect against divide by zero or slight math errors > 1.0
    float cos_theta = 1.0f; 
    if (mag_curr > 0 && mag_base > 0) {
        cos_theta = dot / (mag_curr * mag_base);
    }
    
    // Clamp to valid range for acos [-1, 1]
    if (cos_theta > 1.0f) cos_theta = 1.0f;
    if (cos_theta < -1.0f) cos_theta = -1.0f;
    
    // 4. Calculate Angle
    float angleRad = acos(cos_theta);
    float angleDeg = angleRad * 180.0f / PI;
    
    // Use sign from Pitch (X-axis) to indicate direction if needed
    if (filtered_ax < base_x) {
        angleDeg = -angleDeg;
    }
    
    // Update global
    if (xSemaphoreTake(dataMutex, 0) == pdTRUE) {
      currentAngle = angleDeg;
      xSemaphoreGive(dataMutex);
    }
  }
}

void calibrateAccelerometer(int samples) {
    Serial.println("Calibrating accelerometer (Base Vector)...");
    
    for(int i=0; i<samples; i++) {
        updateAngle(); // Updates filtered values
        delay(10);
    }
    
    // Capture the smoothed vector as our base
    base_x = filtered_ax;
    base_y = filtered_ay;
    base_z = filtered_az;
    
    Serial.print("Base Vector: [");
    Serial.print(base_x); Serial.print(", ");
    Serial.print(base_y); Serial.print(", ");
    Serial.print(base_z); Serial.println("]");
}

#endif // ACCELEROMETER_H
