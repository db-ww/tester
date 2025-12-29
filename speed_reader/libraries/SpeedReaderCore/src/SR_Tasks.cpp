#include "SR_Tasks.h"
#include "globals.h"
#include "SR_SpeedSensor.h"
#include "SR_LCDDisplay.h"
#include "SR_Accelerometer.h"

#if ENABLE_BT
#include <BluetoothSerial.h>
#endif

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
      float ang;
      float vib;
      
      if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        rc = rotationCount;
        totalDistance_miles = (float)rc * distancePerRotation_miles;
        d_miles = totalDistance_miles;
        ang = currentAngle;
        vib = currentVibration;
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
      
      float voltage = (lastAnalog / 4095.0f) * 3.3f;
      Serial.print(" (");
      Serial.print(voltage, 2);
      Serial.print("V)");

      Serial.print(" Angle:");
      Serial.print(ang, 4);
      Serial.print(" Vib:");
      Serial.print(vib, 4);
      Serial.print(" RawAng:");
      Serial.print(rawAngle, 4);
      
      Serial.print(" RawAcc:[");
      Serial.print(debug_raw_x); Serial.print(",");
      Serial.print(debug_raw_y); Serial.print(",");
      Serial.print(debug_raw_z); Serial.print("] Heap:");
      Serial.println(ESP.getFreeHeap());
      
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
    
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void displayTask(void* parameter) {
  unsigned long lastUpdate = 0;
  const unsigned long updateInterval = 500;
  
  while (true) {
    unsigned long now = millis();
    
    if (now - lastUpdate >= updateInterval) {
      lastUpdate = now;
      
      bool shouldShowStats = sessionActive;
      
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
        static unsigned long lastReadyRefresh = 0;
        if (now - lastReadyRefresh > 2000) {
           showReady();
           lastReadyRefresh = now;
        }
      }
    }
    
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}
