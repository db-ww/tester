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
      float speed_m_s = getCurrentSpeed();
      
      unsigned long rc, dist_cm, speed_mm_s;
      if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        rc = rotationCount;
        totalDistance_m = (float)rc * distancePerRotation_m;
        dist_cm = (unsigned long)(totalDistance_m * 100.0f);
        speed_mm_s = (unsigned long)(speed_m_s * 1000.0f);
        xSemaphoreGive(dataMutex);
      }
      
      Serial.print("rot:");
      Serial.print(rc);
      Serial.print(" dist_cm:");
      Serial.print(dist_cm);
      Serial.print(" speed_mm_s:");
      Serial.print(speed_mm_s);
      Serial.print(" D4:");
      Serial.print(lastDigital);
      Serial.print(" D5:");
      Serial.print(lastAnalog);
      Serial.print(" Angle:");
      Serial.print(currentAngle * 100.0f, 4);
      
      // Debug Raw
      extern int16_t debug_raw_x, debug_raw_y, debug_raw_z;
      Serial.print(" Raw:[");
      Serial.print(debug_raw_x); Serial.print(",");
      Serial.print(debug_raw_y); Serial.print(",");
      Serial.print(debug_raw_z); Serial.println("]");
      
      #if ENABLE_BT
      if (haveBT) {
        SerialBT.print(rc);
        SerialBT.print(',');
        SerialBT.print(dist_cm);
        SerialBT.print(',');
        SerialBT.print(speed_mm_s);
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
        float speed_m_s = getCurrentSpeed();
        showSpeed(speed_m_s);
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
