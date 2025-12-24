#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\session.h"
#ifndef SESSION_H
#define SESSION_H

#include "globals.h"
#include "lcd_display.h"

// ===== Session Management =====
void resetSession() {
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    rotationCount = 0;
    totalDistance_miles = 0.0f;
    maxSpeed_mph = 0.0f;
    maxAngle = -180.0f;
    minAngle = 180.0f;
    lastRotationMicros = 0;
    lastRotationIntervalMicros = 0;
    lastRotationTimeMillis = 0;
    xSemaphoreGive(dataMutex);
  }
  Serial.println("Session reset");
}

void startSession(const String& job) {
  resetSession();
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    strncpy(currentJob, job.c_str(), sizeof(currentJob) - 1);
    currentJob[sizeof(currentJob) - 1] = '\0';
    sessionActive = true;
    xSemaphoreGive(dataMutex);
  }
  
  showJob(job);
  Serial.print("Started session for job: ");
  Serial.println(job);
}

void endSession() {
  sessionActive = false;
  showReady();
  Serial.println("Session ended - Ready for new session");
}

#endif // SESSION_H
