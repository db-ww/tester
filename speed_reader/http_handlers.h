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
  
  String job = "";
  String body = server.arg("plain");
  if (body.length() > 0 && body.startsWith("{")) {
    job = getJsonValue(body, "job");
  } else {
    job = server.arg("job");
  }
  
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
  float speed_mph = getCurrentSpeed();
  
  unsigned long rc = 0;
  float dist_miles = 0.0f;
  float max_mph = 0.0f;
  float max_ang = 0.0f;
  float min_ang = 0.0f;
  
  char safeJob[32] = "";
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    rc = rotationCount;
    rc = rotationCount;
    totalDistance_miles = (float)rc * distancePerRotation_miles;
    dist_miles = totalDistance_miles;
    max_mph = maxSpeed_mph;
    max_ang = maxAngle;
    min_ang = minAngle;
    
    // Copy job name while holding mutex
    strncpy(safeJob, currentJob, sizeof(safeJob) - 1);
    safeJob[sizeof(safeJob) - 1] = '\0';
    
    xSemaphoreGive(dataMutex);
  } else {
    // Mutex timeout - return error
    server.send(500, "application/json", "{\"error\":\"Mutex timeout\"}");
    return;
  }
  
  char buf[256];
  // safeJob is already populated safely
  
  snprintf(buf, sizeof(buf), 
    "{\"rotations\":%lu,\"distance_miles\":%.4f,\"speed_mph\":%.2f,\"max_speed\":%.2f,\"angle\":%.1f,\"max_angle\":%.1f,\"min_angle\":%.1f,\"job\":\"%s\"}", 
    rc, dist_miles, speed_mph, max_mph, currentAngle, max_ang, min_ang, safeJob);
  
  server.send(200, "application/json", buf);
}

void handleConfig() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }
  
  String pass = "";
  String body = server.arg("plain");
  bool isJson = (body.length() > 0 && body.startsWith("{"));
  
  if (isJson) {
      pass = getJsonValue(body, "password");
  } else {
      pass = server.arg("password");
  }
  
  if (pass != String(adminPassword)) {
    Serial.print("Auth Failed. Received: '");
    Serial.print(pass);
    Serial.print("' Expected: '");
    Serial.print(adminPassword);
    Serial.println("'");
    server.send(401, "application/json", "{\"error\":\"Unauthorized\"}");
    return;
  }
  
  // Update Configs
  if (isJson) {
      String val;
      val = getJsonValue(body, "ssid"); if (val.length() > 0) strncpy(wifiSSID, val.c_str(), sizeof(wifiSSID) - 1);
      val = getJsonValue(body, "wifi_password"); if (val.length() > 0) strncpy(wifiPassword, val.c_str(), sizeof(wifiPassword) - 1);
      val = getJsonValue(body, "name"); if (val.length() > 0) strncpy(deviceName, val.c_str(), sizeof(deviceName) - 1);
      
      val = getJsonValue(body, "speed_offset"); if (val.length() > 0) speedOffset = val.toFloat();
      val = getJsonValue(body, "speed_scale"); if (val.length() > 0) speedScale = val.toFloat();
      val = getJsonValue(body, "pulses_per_rotation"); if (val.length() > 0) pulsesPerRotation = val.toInt();
      val = getJsonValue(body, "distance_offset"); if (val.length() > 0) distanceOffset = val.toFloat();
      val = getJsonValue(body, "angle_offset"); if (val.length() > 0) angleOffset = val.toFloat();
      val = getJsonValue(body, "accel_offset"); if (val.length() > 0) accelOffset = val.toFloat();
      val = getJsonValue(body, "accel_scale"); if (val.length() > 0) accelScale = val.toFloat();
      
      val = getJsonValue(body, "new_password");
      if (val.length() > 0 && val.length() < 32) {
         strncpy(adminPassword, val.c_str(), sizeof(adminPassword) - 1);
      }
  } else {
      if (server.hasArg("ssid")) strncpy(wifiSSID, server.arg("ssid").c_str(), sizeof(wifiSSID) - 1);
      if (server.hasArg("wifi_password")) strncpy(wifiPassword, server.arg("wifi_password").c_str(), sizeof(wifiPassword) - 1);
      if (server.hasArg("name")) strncpy(deviceName, server.arg("name").c_str(), sizeof(deviceName) - 1);
      
      if (server.hasArg("speed_offset")) speedOffset = server.arg("speed_offset").toFloat();
      if (server.hasArg("speed_scale")) speedScale = server.arg("speed_scale").toFloat();
      if (server.hasArg("pulses_per_rotation")) pulsesPerRotation = server.arg("pulses_per_rotation").toInt();
      if (server.hasArg("distance_offset")) distanceOffset = server.arg("distance_offset").toFloat();
      if (server.hasArg("angle_offset")) angleOffset = server.arg("angle_offset").toFloat();
      if (server.hasArg("accel_offset")) accelOffset = server.arg("accel_offset").toFloat();
      if (server.hasArg("accel_scale")) accelScale = server.arg("accel_scale").toFloat();
      
      if (server.hasArg("new_password")) {
        String newPass = server.arg("new_password");
        if (newPass.length() > 0 && newPass.length() < 32) {
          strncpy(adminPassword, newPass.c_str(), sizeof(adminPassword) - 1);
        }
      }
  }
  
  // Save to SPIFFS
  saveConfig();
  
  // Build Response
  String json = "{";
  json += "\"status\":\"ok\",";
  json += "\"device\":{";
  json += "\"name\":\"" + String(deviceName) + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"uptime_ms\":" + String(millis()) + ",";
  json += "\"heap_free\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"chip_model\":\"" + String(ESP.getChipModel()) + "\"";
  json += "},";
  json += "\"config\":{";
  json += "\"ssid\":\"" + String(wifiSSID) + "\",";
  json += "\"wifi_password\":\"(hidden)\",";
  json += "\"speed_offset\":" + String(speedOffset) + ",";
  json += "\"speed_scale\":" + String(speedScale) + ",";
  json += "\"pulses_per_rotation\":" + String(pulsesPerRotation) + ",";
  json += "\"distance_offset\":" + String(distanceOffset) + ",";
  json += "\"angle_offset\":" + String(angleOffset) + ",";
  json += "\"accel_offset\":" + String(accelOffset) + ",";
  json += "\"accel_scale\":" + String(accelScale);
  json += "}";
  json += "}";
  
  server.send(200, "application/json", json);
}

#endif // ENABLE_HTTP

#endif // HTTP_HANDLERS_H
