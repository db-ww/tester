#include "SR_Session.h"
#include "SR_LCDDisplay.h"
#include "globals.h"

// ===== Session Management =====
void resetSession() {
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    rotationCount = 0;
    maxSpeed_mph = 0.0f;
    lastRotationMicros = 0;
    lastRotationIntervalMicros = 0;
    lastRotationTimeMillis = 0;
    totalDistance_miles = 0.0f;
    maxAngle = -180.0f;
    minAngle = 180.0f;
    maxVibration = 0.0f;
    xSemaphoreGive(dataMutex);
  }
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
}

void endSession() {
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    sessionActive = false;
    currentJob[0] = '\0'; 
    xSemaphoreGive(dataMutex);
  }
  showReady();
}
