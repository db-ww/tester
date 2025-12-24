#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "globals.h"

// ===== LCD Display Functions =====
void updateLCD(const char* line1, const char* line2) {
  if (xSemaphoreTake(lcdMutex, portMAX_DELAY) != pdTRUE) return;

  // Pad strings to 16 chars to overwrite old content without clearing
  char padded1[17];
  char padded2[17];
  
  snprintf(padded1, sizeof(padded1), "%-16s", line1);
  if (line2 != NULL) {
    snprintf(padded2, sizeof(padded2), "%-16s", line2);
  } else {
    snprintf(padded2, sizeof(padded2), "                ");
  }
  
  lcd.setCursor(0, 0);
  lcd.print(padded1);
  lcd.setCursor(0, 1);
  lcd.print(padded2);
  
  xSemaphoreGive(lcdMutex);
}

void showReady() {
  updateLCD("Job:None", "Ready");
}

void showJob(const String& job) {
  char line1[17];
  // Truncate job to 5 chars to fit "Job:xxxxx Start" (16 chars)
  char safeJob[6];
  strncpy(safeJob, job.c_str(), 5);
  safeJob[5] = '\0';
  
  snprintf(line1, sizeof(line1), "Job:%s Start", safeJob);
  updateLCD(line1, "R:0 S:0");
}

void showSpeed(float speed_mph) {
  char line1[17], line2[17];
  
  unsigned long rc = 0;
  bool active = false;
  char jobNameBuf[32] = "";
  float angle = 0.0f;
  
  // Get shared data safely
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    rc = rotationCount;
    active = sessionActive;
    angle = currentAngle;
    strncpy(jobNameBuf, currentJob, sizeof(jobNameBuf) - 1);
    jobNameBuf[sizeof(jobNameBuf) - 1] = '\0';
    xSemaphoreGive(dataMutex);
  }
  
  float dist_miles = (float)rc * distancePerRotation_miles;
  
  // Line 1: Job + Status
  const char* status = active ? "Run " : "Free";
  const char* jobName = (strlen(jobNameBuf) > 0) ? jobNameBuf : "None";
  
  // Format with strict width limits to prevent overflow
  // %.7s truncates job name to 7 chars max
  snprintf(line1, sizeof(line1), "Job:%.7s %.4s", jobName, status);
  
  // Line 2: Dist + Speed + Angle
  // Line 2: Speed + Angle (Distance removed to fit 2 decimal speed)
  // Example: S:12.34 A:12.3
  snprintf(line2, sizeof(line2), "S:%.2f A:%.1f", speed_mph, angle);
  
  updateLCD(line1, line2);
}

#endif // LCD_DISPLAY_H
