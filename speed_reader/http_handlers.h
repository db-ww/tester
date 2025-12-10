#ifndef HTTP_HANDLERS_H
#define HTTP_HANDLERS_H

#include "globals.h"
#include "session.h"
#include "speed_sensor.h"

#if ENABLE_HTTP

// ===== HTTP Handlers =====
void handleRoot() {
  server.send(200, "text/plain", "HTTP server OK.\\nTry /readings or /start");
}

void handleStart() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed. Use POST.");
    return;
  }
  
  String job = server.arg("job");
  if (job.length() == 0) {
    server.send(400, "application/json", "{\"error\":\"Missing 'job' parameter\"}");
    return;
  }
  
  // Validate job name length to prevent buffer issues
  if (job.length() > 31) {
    server.send(400, "application/json", "{\"error\":\"Job name too long (max 31 chars)\"}");
    return;
  }
  
  startSession(job);
  
  char response[128];
  snprintf(response, sizeof(response), "{\"status\":\"started\",\"job\":\"%s\"}", job.c_str());
  server.send(200, "application/json", response);
}

void handleReadings() {
  float speed_m_s = getCurrentSpeed();
  
  unsigned long rc = 0, dist_cm = 0, speed_mm_s = 0, max_speed_mm_s = 0;
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    rc = rotationCount;
    totalDistance_m = (float)rc * distancePerRotation_m;
    dist_cm = (unsigned long)(totalDistance_m * 100.0f);
    speed_mm_s = (unsigned long)(speed_m_s * 1000.0f);
    max_speed_mm_s = (unsigned long)(maxSpeed_m_s * 1000.0f);
    xSemaphoreGive(dataMutex);
  } else {
    // Mutex timeout - return error
    server.send(500, "application/json", "{\"error\":\"Mutex timeout\"}");
    return;
  }
  
  char buf[256];
  // Safely truncate job name if too long to prevent buffer overflow
  char safeJob[32];
  strncpy(safeJob, currentJob.c_str(), sizeof(safeJob) - 1);
  safeJob[sizeof(safeJob) - 1] = '\0';
  
  snprintf(buf, sizeof(buf), 
    "{\"digital\":%d,\"analog\":%d,\"rotations\":%lu,\"distance_cm\":%lu,\"speed_mm_s\":%lu,\"max_speed\":%lu,\"angle\":%.1f,\"job\":\"%s\"}", 
    lastDigital, lastAnalog, rc, dist_cm, speed_mm_s, max_speed_mm_s, currentAngle, safeJob);
  
  server.send(200, "application/json", buf);
}

#endif // ENABLE_HTTP

#endif // HTTP_HANDLERS_H
