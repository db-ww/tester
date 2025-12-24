#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\accelerometer.h"
#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Wire.h>
#include <math.h>
#include "globals.h"

#define ACCEL_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_XOUT_H 0x3B

bool isAccelConnected = false;

// Kalman/Complementary filter constants
const float FILTER_ALPHA = 0.98f; // Trust Gyro 98%, Accel 2%

// Globals for calibration and state
float gyro_x_offset = 0.0f;
float gyro_y_offset = 0.0f;
float gyro_z_offset = 0.0f;

float rawAngle = 0.0f;

// Reference "Zero" vector (Gravity when calibrated)
float base_x = 0.0f;
float base_y = 0.0f;
float base_z = 1.0f;

// Current Estimated Gravity Vector (Fused)
float est_x = 0.0f;
float est_y = 0.0f;
float est_z = 1.0f;

unsigned long last_update_micros = 0;


bool writeAccelRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ACCEL_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return (Wire.endTransmission() == 0);
}

void initAccelerometer() {
  Serial.println("Initializing GY-521 (MPU6050)...");
  
  // Initialize I2C with defined pins
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // 400kHz fast mode
  Wire.setTimeOut(3000); // Timeout in 3000ms to prevent infinite hangs
  
  // Check if device is reachable
  Wire.beginTransmission(ACCEL_ADDR);
  if (Wire.endTransmission() != 0) {
      Serial.println("GY-521 (MPU6050) not found. Accelerometer disabled.");
      isAccelConnected = false;
      return;
  }
  
  // 1. Wake up
  if (!writeAccelRegister(MPU6050_PWR_MGMT_1, 0x01)) { 
       isAccelConnected = false;
       return;
  }
  delay(100);

  // 2. DLPF - Mode 3 (44Hz bandwidth)
  if (!writeAccelRegister(MPU6050_CONFIG, 0x03)) { 
       isAccelConnected = false;
       return;
  }

  // 3. Gyro Config - 250 dps
  writeAccelRegister(MPU6050_GYRO_CONFIG, 0x00);
  
  isAccelConnected = true;
  last_update_micros = micros();
  Serial.println("GY-521 init done.");
}

// Debug output
int16_t debug_raw_x = 0;
int16_t debug_raw_y = 0;
int16_t debug_raw_z = 0;

void updateAngle() {
  if (!isAccelConnected) return;

  unsigned long now = micros();
  float dt = (now - last_update_micros) / 1000000.0f;
  last_update_micros = now;
  if (dt > 1.0f || dt <= 0.0f) dt = 0.0f;

  Wire.beginTransmission(ACCEL_ADDR);
  Wire.write(MPU6050_ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) return;
  
  Wire.requestFrom((uint8_t)ACCEL_ADDR, (uint8_t)14);
  if (Wire.available() >= 14) {
    int16_t ax_raw = (Wire.read() << 8) | Wire.read();
    int16_t ay_raw = (Wire.read() << 8) | Wire.read();
    int16_t az_raw = (Wire.read() << 8) | Wire.read();
    Wire.read(); Wire.read(); // temp
    int16_t gx_raw = (Wire.read() << 8) | Wire.read();
    int16_t gy_raw = (Wire.read() << 8) | Wire.read();
    int16_t gz_raw = (Wire.read() << 8) | Wire.read();
    
    debug_raw_x = ax_raw;
    debug_raw_y = ay_raw;
    debug_raw_z = az_raw;
    
    // 1. Get Measured Acceleration Vector (Normalized)
    float ax = ax_raw;
    float ay = ay_raw;
    float az = az_raw;
    float norm_a = sqrt(ax*ax + ay*ay + az*az);
    if (norm_a > 0.0f) {
      ax /= norm_a;
      ay /= norm_a;
      az /= norm_a;
    }

    // 2. Get Gyro rates (rad/s)
    float gx = ((gx_raw / 131.0f) - gyro_x_offset) * DEG_TO_RAD;
    float gy = ((gy_raw / 131.0f) - gyro_y_offset) * DEG_TO_RAD;
    float gz = ((gz_raw / 131.0f) - gyro_z_offset) * DEG_TO_RAD;
    
    // 3. Integrate Gyro to rotate Estimated Gravity
    // V_new = V_old + (Omega x V_old) * dt
    // Cross Product:
    // x = vy*wz - vz*wy
    // y = vz*wx - vx*wz
    // z = vx*wy - vy*wx
    float dx = (est_y * gz - est_z * gy) * dt;
    float dy = (est_z * gx - est_x * gz) * dt;
    float dz = (est_x * gy - est_y * gx) * dt;
    
    float pred_x = est_x + dx;
    float pred_y = est_y + dy;
    float pred_z = est_z + dz;
    
    // Normalize predicted vector
    float norm_p = sqrt(pred_x*pred_x + pred_y*pred_y + pred_z*pred_z);
    if (norm_p > 0.0f) {
      pred_x /= norm_p;
      pred_y /= norm_p;
      pred_z /= norm_p;
    }
    
    // 4. Complementary Filter Fusion (Correct with Accel)
    // Est = Alpha * Pred + (1-Alpha) * Meas
    // Note: Accel is very noisy, so keep Alpha high
    const float ALPHA = 0.99f;
    est_x = pred_x * ALPHA + ax * (1.0f - ALPHA);
    est_y = pred_y * ALPHA + ay * (1.0f - ALPHA);
    est_z = pred_z * ALPHA + az * (1.0f - ALPHA);
    
    // Normalize Result
    float norm_e = sqrt(est_x*est_x + est_y*est_y + est_z*est_z);
    if (norm_e > 0.0f) {
      est_x /= norm_e;
      est_y /= norm_e;
      est_z /= norm_e;
    }
    
    // 5. Calculate Angles
    // Raw Angle (from instantaneous Accel)
    float rawDot = ax*base_x + ay*base_y + az*base_z;
    if (rawDot > 1.0f) rawDot = 1.0f;
    if (rawDot < -1.0f) rawDot = -1.0f;
    rawAngle = acos(rawDot) * RAD_TO_DEG;
    
    // Filtered Angle
    float dot = est_x*base_x + est_y*base_y + est_z*base_z;
    if (dot > 1.0f) dot = 1.0f;
    if (dot < -1.0f) dot = -1.0f;
    
    float angleRad = acos(dot);
    float angleDeg = angleRad * RAD_TO_DEG;
    
    // Apply Offset
    angleDeg += angleOffset;
    
    // Post-Smoothing (EMA)
    static float smoothedAngle = 0.0f;
    const float SMOOTH_ALPHA = 0.1f; // Lower = more smooth, slower response
    smoothedAngle = smoothedAngle * (1.0f - SMOOTH_ALPHA) + angleDeg * SMOOTH_ALPHA;
    
    // Deadband for "Zeroing out"
    float finalAngle = smoothedAngle;
    if (finalAngle < 0.2f && finalAngle > -0.2f) finalAngle = 0.0f;
    
    if (xSemaphoreTake(dataMutex, 0) == pdTRUE) {
      currentAngle = finalAngle;
      if (currentAngle > maxAngle) maxAngle = currentAngle;
      if (currentAngle < minAngle) minAngle = currentAngle;
      xSemaphoreGive(dataMutex);
    }
  }
}

void calibrateAccelerometer(int samples) {
    if (!isAccelConnected) return;

    Serial.println("Calibrating 3D Vector...");
    
    // Warmup
    for(int i=0; i<100; i++) {
        updateAngle();
        delay(5);
    }
    
    long gxs=0, gys=0, gzs=0;
    float sum_ax=0, sum_ay=0, sum_az=0;
    int n = 200;
    
    for (int i=0; i<n; i++) {
        Wire.beginTransmission(ACCEL_ADDR);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)ACCEL_ADDR, (uint8_t)14);
        if (Wire.available() >= 14) {
             int16_t ax = (Wire.read() << 8) | Wire.read();
             int16_t ay = (Wire.read() << 8) | Wire.read();
             int16_t az = (Wire.read() << 8) | Wire.read();
             Wire.read(); Wire.read();
             int16_t gx = (Wire.read() << 8) | Wire.read();
             int16_t gy = (Wire.read() << 8) | Wire.read();
             int16_t gz = (Wire.read() << 8) | Wire.read();
             
             gxs += gx;
             gys += gy;
             gzs += gz;
             
             sum_ax += ax;
             sum_ay += ay;
             sum_az += az;
        }
        delay(5);
    }
    
    gyro_x_offset = (float)gxs / n / 131.0f;
    gyro_y_offset = (float)gys / n / 131.0f;
    gyro_z_offset = (float)gzs / n / 131.0f;
    
    // Average Accel Vector
    base_x = sum_ax / n;
    base_y = sum_ay / n;
    base_z = sum_az / n;
    
    // Normalize Base
    float norm = sqrt(base_x*base_x + base_y*base_y + base_z*base_z);
    if (norm > 0) {
      base_x /= norm;
      base_y /= norm;
      base_z /= norm;
    } else {
      base_z = 1.0f; // Fallback
    }

    // Initialize Estimated Vector to Base
    est_x = base_x;
    est_y = base_y;
    est_z = base_z;
    
    Serial.println("Calibration Done.");
    Serial.print("Base Vector: ");
    Serial.print(base_x); Serial.print(", ");
    Serial.print(base_y); Serial.print(", ");
    Serial.println(base_z);
}

#endif // ACCELEROMETER_H
