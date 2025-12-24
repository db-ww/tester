#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\tasks.h"
#ifndef TASKS_H
#define TASKS_H

#include "globals.h"
#include "speed_sensor.h"
#include "lcd_display.h"

// ===== FreeRTOS Tasks =====
void sensorTask(void* parameter) {
  unsigned long lastPrint = 0;
  const unsigned long printInterval = 1000;
  
  while (true) {
    unsigned long now = millis();
    
    // Read ADC
    if (now - lastAdcRead >= readIntervalMs) {
      lastAdcRead = now;
      lastAnalog = analogRead(D5_ANALOG);
    }
    
    // Read digital
    if (now - lastDigitalRead >= readIntervalMs) {
      lastDigitalRead = now;
      lastDigital = digitalRead(D4_DIGITAL);
    }

    // Read Accelerometer
    updateAngle();
    
    // Periodic debug print
    if (now - lastPrint >= printInterval) {
      lastPrint = now;
      float speed_mph = getCurrentSpeed();
      
      unsigned long rc;
      float d_miles;
      
      if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        rc = rotationCount;
        rc = rotationCount;
        totalDistance_miles = (float)rc * distancePerRotation_miles;
        d_miles = totalDistance_miles;
        xSemaphoreGive(dataMutex);
      }
      
      Serial.print("rot:");
      Serial.print(rc);
      Serial.print(" dist_mi:");
      Serial.print(d_miles, 2);
      Serial.print(" speed_mph:");
      Serial.print(speed_mph, 2);
      Serial.print(" D4(Pin");
      Serial.print(D4_DIGITAL);
      Serial.print("):");
      Serial.print(lastDigital);
      Serial.print(" D5(Pin");
      Serial.print(D5_ANALOG);
      Serial.print("):");
      Serial.print(lastAnalog);
      
      // Calculate and print voltage for debugging
      float voltage = (lastAnalog / 4095.0f) * 3.3f; // Assumes 11dB attenuation
      Serial.print(" (");
      Serial.print(voltage, 2);
      Serial.print("V)");

      Serial.print(" Angle:");
      Serial.print(currentAngle, 4);
      
      // Access raw angle (no mutex needed as it's just for debug/stateless)
      extern float rawAngle;
      Serial.print(" RawAng:");
      Serial.print(rawAngle, 4);
      
      // Debug Raw
      extern int16_t debug_raw_x, debug_raw_y, debug_raw_z;
      Serial.print(" RawAcc:[");
      Serial.print(debug_raw_x); Serial.print(",");
      Serial.print(debug_raw_y); Serial.print(",");
      Serial.print(debug_raw_z); Serial.println("]");
      
      #if ENABLE_BT
      if (haveBT) {
        SerialBT.print(rc);
        SerialBT.print(',');
        SerialBT.print(d_miles);
        SerialBT.print(',');
        SerialBT.print(speed_mph);
        SerialBT.print(',');
        SerialBT.println(lastAnalog);
      }
      #endif
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void displayTask(void* parameter) {
  unsigned long lastUpdate = 0;
  const unsigned long updateInterval = 500;
  
  while (true) {
    unsigned long now = millis();
    
    if (now - lastUpdate >= updateInterval) {
      lastUpdate = now;
      
      // Show stats if session is active OR if we have rotations (Free Run mode)
      bool shouldShowStats = sessionActive;
      
      // Check for free run (movement without job)
      if (!shouldShowStats) {
        if (xSemaphoreTake(dataMutex, 0) == pdTRUE) {
          if (rotationCount > 0) shouldShowStats = true;
          xSemaphoreGive(dataMutex);
        }
      }

      if (shouldShowStats) {
        float speed_mph = getCurrentSpeed();
        showSpeed(speed_mph);
      } else {
        // Only refresh Ready screen occasionally to prevent flickering 
        // if it was already showing ready, but here we just leave it.
        // Or we can reinforce it every few seconds.
        static unsigned long lastReadyRefresh = 0;
        if (now - lastReadyRefresh > 2000) {
           showReady();
           lastReadyRefresh = now;
        }
      }
    }
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

#endif // TASKS_H
