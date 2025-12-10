#ifndef SPEED_SENSOR_H
#define SPEED_SENSOR_H

#include "globals.h"
#include "session.h"

// ===== Interrupt Handler =====
void IRAM_ATTR onRotation() {
  unsigned long t = micros();
  const unsigned long debounceUs = 50000UL; // Increased to 50ms to prevent double counting
  
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
  
  float speed_m_s = 0.0f;
  if (intervalUs > 0) {
    speed_m_s = distancePerRotation_m * 1e6f / (float)intervalUs;
    
    // Update max speed
    if (speed_m_s > maxSpeed_m_s) {
      maxSpeed_m_s = speed_m_s;
    }
  }
  
  return speed_m_s;
}

#endif // SPEED_SENSOR_H
