#include "SR_HTTPHandlers.h"
#include "SR_Session.h"
#include "SR_SpeedSensor.h"
#include "SR_WiFiLoader.h"
#include <WiFi.h>

#if ENABLE_HTTP

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->setHeader("Content-Type", "text/plain");
  res->println("HTTPS/HTTP server OK.\nTry /readings or /start");
}

void handleStart(HTTPRequest * req, HTTPResponse * res) {
  if (req->getMethod() != "POST") {
    res->setStatusCode(405);
    res->setStatusText("Method Not Allowed");
    return;
  }
  
  String job = "";
  char buffer[256];
  size_t len = req->readChars(buffer, 255);
  buffer[len] = '\0';
  String body = String(buffer);
  
  ResourceParameters *params = req->getParams();

  if (body.length() > 0 && body.startsWith("{")) {
    job = getJsonValue(body, "job");
  } else {
    if (params->isQueryParameterSet("job")) {
       std::string jstr;
       params->getQueryParameter("job", jstr);
       job = String(jstr.c_str());
    }
  }
  
  if (job.length() == 0) {
    res->setStatusCode(400);
    res->setHeader("Content-Type", "application/json");
    res->print("{\"error\":\"Missing 'job' parameter\"}");
    return;
  }
  
  if (job.length() > 31) {
    res->setStatusCode(400);
    res->setHeader("Content-Type", "application/json");
    res->print("{\"error\":\"Job name too long (max 31 chars)\"}");
    return;
  }
  
  startSession(job);
  
  char response[128];
  snprintf(response, sizeof(response), "{\"status\":\"started\",\"job\":\"%s\"}", job.c_str());
  res->setHeader("Content-Type", "application/json");
  res->print(response);
}

void handleReadings(HTTPRequest * req, HTTPResponse * res) {
  float speed_mph = getCurrentSpeed();
  
  unsigned long rc = 0;
  float dist_miles = 0.0f;
  float max_mph = 0.0f;
  float max_ang = 0.0f;
  float min_ang = 0.0f;
  float vib = 0.0f;
  float max_vib = 0.0f;
  
  char safeJob[32] = "";
  
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    rc = rotationCount;
    totalDistance_miles = (float)rc * distancePerRotation_miles;
    dist_miles = totalDistance_miles;
    max_mph = maxSpeed_mph;
    max_ang = maxAngle;
    min_ang = minAngle;
    vib = currentVibration;
    max_vib = maxVibration;
    strncpy(safeJob, currentJob, sizeof(safeJob) - 1);
    safeJob[sizeof(safeJob) - 1] = '\0';
    xSemaphoreGive(dataMutex);
  } else {
    res->setStatusCode(500);
    res->setHeader("Content-Type", "application/json");
    res->print("{\"error\":\"Mutex timeout\"}");
    return;
  }
  
  char buf[256];
  snprintf(buf, sizeof(buf), 
    "{\"rotations\":%lu,\"distance_miles\":%.4f,\"speed_mph\":%.2f,\"max_speed\":%.2f,\"angle\":%.1f,\"max_angle\":%.1f,\"min_angle\":%.1f,\"vibration\":%.3f,\"max_vibration\":%.3f,\"job\":\"%s\"}", 
    rc, dist_miles, speed_mph, max_mph, currentAngle, max_ang, min_ang, vib, max_vib, safeJob);
  
  res->setHeader("Content-Type", "application/json");
  res->print(buf);
}

void handleConfig(HTTPRequest * req, HTTPResponse * res) {
  if (req->getMethod() != "POST") {
    res->setStatusCode(405);
    res->setStatusText("Method Not Allowed");
    return;
  }
  
  char buffer[1024]; 
  size_t len = req->readChars(buffer, 1023);
  buffer[len] = '\0';
  String body = String(buffer);
  
  bool isJson = (body.length() > 0 && body.startsWith("{"));
  ResourceParameters *params = req->getParams();
  
  auto getParam = [&](const char* key, String &target) {
     if (params->isQueryParameterSet(key)) {
         std::string s; params->getQueryParameter(key, s);
         target = String(s.c_str());
     }
  };

  String pass = "";
  if (isJson) {
      pass = getJsonValue(body, "device_password");
  } else {
      getParam("device_password", pass);
  }

  if (pass != String(devicePassword)) {
    res->setStatusCode(401);
    res->setHeader("Content-Type", "application/json");
    res->print("{\"error\":\"Unauthorized. Invalid 'password' in request body.\"}");
    return;
  }
   
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
      val = getJsonValue(body, "vibration_offset"); if (val.length() > 0) vibrationOffset = val.toFloat();
      
      val = getJsonValue(body, "api_key");
      if (val.length() > 0 && val.length() < 64) {
         strncpy(apiKey, val.c_str(), sizeof(apiKey) - 1);
      }
      
      val = getJsonValue(body, "device_password");
      if (val.length() > 0 && val.length() < 32) {
         strncpy(devicePassword, val.c_str(), sizeof(devicePassword) - 1);
      }
      
      val = getJsonValue(body, "register_url");
      if (val.length() > 0) strncpy(registerUrl, val.c_str(), sizeof(registerUrl) - 1);
      
      val = getJsonValue(body, "station");
      if (val.length() > 0) strncpy(station, val.c_str(), sizeof(station) - 1);
      
      val = getJsonValue(body, "use_https");
      if (val.length() > 0) useHTTPS = (val == "true" || val == "1");

  } else {
      String s;
      getParam("ssid", s); if(s.length()>0) strncpy(wifiSSID, s.c_str(), sizeof(wifiSSID)-1);
      getParam("wifi_password", s); if(s.length()>0) strncpy(wifiPassword, s.c_str(), sizeof(wifiPassword)-1);
      getParam("name", s); if(s.length()>0) strncpy(deviceName, s.c_str(), sizeof(deviceName)-1);
      
      getParam("speed_offset", s); if(s.length()>0) speedOffset = s.toFloat();
      getParam("speed_scale", s); if(s.length()>0) speedScale = s.toFloat();
      getParam("pulses_per_rotation", s); if(s.length()>0) pulsesPerRotation = s.toInt();
      getParam("distance_offset", s); if(s.length()>0) distanceOffset = s.toFloat();
      getParam("angle_offset", s); if(s.length()>0) angleOffset = s.toFloat();
      getParam("accel_offset", s); if(s.length()>0) accelOffset = s.toFloat();
      getParam("accel_scale", s); if(s.length()>0) accelScale = s.toFloat();
      getParam("vibration_offset", s); if(s.length()>0) vibrationOffset = s.toFloat();
      
      getParam("api_key", s);
      if (s.length() > 0 && s.length() < 64) {
        strncpy(apiKey, s.c_str(), sizeof(apiKey) - 1);
      }
      
      getParam("device_password", s);
      if (s.length() > 0 && s.length() < 32) {
        strncpy(devicePassword, s.c_str(), sizeof(devicePassword) - 1);
      }

      getParam("register_url", s);
      if (s.length() > 0) strncpy(registerUrl, s.c_str(), sizeof(registerUrl) - 1);

      getParam("station", s);
      if (s.length() > 0) strncpy(station, s.c_str(), sizeof(station) - 1);

      getParam("use_https", s);
      if (s.length() > 0) useHTTPS = (s == "true" || s == "1");
  }
  
  saveConfig();
  
  String json = "{";
  json += "\"status\":\"ok\",";
  json += "\"device\":{";
  json += "\"name\":\"" + String(deviceName) + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"uptime_min\":" + String(millis() / 60000.0f, 2) + ",";
  json += "\"heap_free\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"chip_model\":\"" + String(ESP.getChipModel()) + "\",";
  json += "\"use_https\":" + String(useHTTPS ? "true" : "false");
  json += "},";
  json += "\"security\":{";
  json += "\"https_enabled\":" + String(useHTTPS ? "true" : "false") + ",";
  json += "\"server_running\":" + String(serverStarted ? "true" : "false") + ",";
  json += "\"wifi_password\":\"(hidden)\",";
  json += "\"device_password\":\"(hidden)\",";
  
  // Mask API key but show first 4 chars for identification
  String maskedKey = "";
  if (strlen(apiKey) >= 4) {
    char firstFour[5];
    strncpy(firstFour, apiKey, 4);
    firstFour[4] = '\0';
    maskedKey = String(firstFour) + "****";
  } else {
    maskedKey = "****";
  }
  json += "\"api_key\":\"" + maskedKey + "\",";
  if (cert) {
    json += "\"cert_size\":" + String(cert->getCertLength()) + ",";
    json += "\"key_size\":" + String(cert->getPKLength());
  } else {
    json += "\"cert_size\":0,";
    json += "\"key_size\":0";
  }
  json += "},";
  json += "\"config\":{";
  json += "\"ssid\":\"" + String(wifiSSID) + "\",";
  json += "\"speed_offset\":" + String(speedOffset) + ",";
  json += "\"speed_scale\":" + String(speedScale) + ",";
  json += "\"pulses_per_rotation\":" + String(pulsesPerRotation) + ",";
  json += "\"distance_offset\":" + String(distanceOffset) + ",";
  json += "\"angle_offset\":" + String(angleOffset) + ",";
  json += "\"accel_offset\":" + String(accelOffset) + ",";
  json += "\"accel_scale\":" + String(accelScale) + ",";
  json += "\"register_url\":\"" + String(registerUrl) + "\",";
  json += "\"station\":\"" + String(station) + "\",";
  json += "\"vibration_offset\":" + String(vibrationOffset);
  json += "}";
  json += "}";
  
  res->setHeader("Content-Type", "application/json");
  res->print(json);
}

void middlewareAuthentication(HTTPRequest * req, HTTPResponse * res, std::function<void()> next) {
  // All requests require X-API-Key header
  std::string headerKey = req->getHeader("X-API-Key");
  
  if (headerKey == std::string(apiKey)) {
    next();
  } else {
    res->setStatusCode(401);
    res->setHeader("Content-Type", "application/json");
    res->print("{\"error\":\"Unauthorized. Missing or invalid X-API-Key header.\"}");
  }
}

void registerRoutes(HTTPServer *srv) {
  // Register authentication middleware globally
  srv->addMiddleware(&middlewareAuthentication);

  ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeStart = new ResourceNode("/start", "POST", &handleStart);
  ResourceNode * nodeReadings = new ResourceNode("/readings", "GET", &handleReadings);
  ResourceNode * nodeConfig = new ResourceNode("/config", "POST", &handleConfig);

  srv->registerNode(nodeRoot);
  srv->registerNode(nodeStart);
  srv->registerNode(nodeReadings);
  srv->registerNode(nodeConfig);
}

#endif
