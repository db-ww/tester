#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\speed_sensor.h"
#ifndef SPEED_SENSOR_H
#define SPEED_SENSOR_H

#include "globals.h"
#include "session.h"

// ===== Interrupt Handler =====
void IRAM_ATTR onRotation() {
  unsigned long t = micros();
  const unsigned long debounceUs = 5000UL; // Reduced to 5ms to allow speeds up to ~124mph
  
  if (lastRotationMicros != 0) {
    unsigned long dt = t - lastRotationMicros;
    if (dt < debounceUs) return;
    lastRotationIntervalMicros = dt;
  }
  
  lastRotationMicros = t;
  lastRotationTimeMillis = millis();
  rotationCount++;
}

// ===== Speed Calculation =====
float getCurrentSpeed() {
  unsigned long now_ms = millis();
  unsigned long lastRot_ms = lastRotationTimeMillis;
  unsigned long intervalUs = lastRotationIntervalMicros;
  
  // Check if speed has timed out
  if (lastRot_ms > 0 && (now_ms - lastRot_ms) > speedTimeoutMs) {
    intervalUs = 0;
    if (sessionActive) {
      endSession();
    }
  }
  
  float speed_mph = 0.0f;
  if (intervalUs > 0) {
    // Miles per microsecond * 3.6e9 microseconds per hour
    // Wait: 1 hour = 3600 sec = 3,600,000,000 us.
    // Speed (mph) = (Miles / us) * (us / hour)
    //             = (distancePerRotation_miles / intervalUs) * 3,600,000,000
    speed_mph = (distancePerRotation_miles * 3.6e9f) / (float)intervalUs;
    speed_mph *= speedScale;
    speed_mph += speedOffset;
    
    // Ensure non-negative
    if (speed_mph < 0) speed_mph = 0;
    
    // Update max speed
    if (speed_mph > maxSpeed_mph) {
      maxSpeed_mph = speed_mph;
    }
  }
  
  return speed_mph;
}

#endif // SPEED_SENSOR_H
