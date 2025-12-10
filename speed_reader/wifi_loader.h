#ifndef WIFI_LOADER_H
#define WIFI_LOADER_H

#include <SPIFFS.h>
#include "globals.h"

// ===== WiFi Configuration Loader =====
bool loadWiFiConfig() {
  Serial.print("Loading WiFi config from SPIFFS...");
  
  if (!SPIFFS.begin(true)) {
    Serial.println(" FAILED to mount SPIFFS");
    return false;
  }
  
  if (!SPIFFS.exists("/config.json")) {
    Serial.println(" config.json not found");
    strncpy(wifiSSID, "BAYSAN", sizeof(wifiSSID) - 1);
    strncpy(wifiPassword, "timetowork", sizeof(wifiPassword) - 1);
    Serial.println("Using default WiFi credentials");
    return true;
  }
  
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println(" Failed to open config.json");
    return false;
  }
  
  // Read file into buffer to avoid String fragmentation
  size_t fileSize = file.size();
  if (fileSize > 1024) {  // Sanity check
    Serial.println(" Config file too large");
    file.close();
    return false;
  }
  
  char* buffer = (char*)malloc(fileSize + 1);
  if (!buffer) {
    Serial.println(" Failed to allocate buffer");
    file.close();
    return false;
  }
  
  file.readBytes(buffer, fileSize);
  buffer[fileSize] = '\0';
  file.close();
  
  // Extract SSID using char* operations
  char* ssid_start = strstr(buffer, "\"ssid\":");
  if (ssid_start) {
    ssid_start = strchr(ssid_start + 7, '"');
    if (ssid_start) {
      ssid_start++;
      char* ssid_end = strchr(ssid_start, '"');
      if (ssid_end) {
        size_t len = ssid_end - ssid_start;
        if (len < sizeof(wifiSSID)) {
          memcpy(wifiSSID, ssid_start, len);
          wifiSSID[len] = '\0';
        }
      }
    }
  }
  
  // Extract password
  char* pwd_start = strstr(buffer, "\"password\":");
  if (pwd_start) {
    pwd_start = strchr(pwd_start + 11, '"');
    if (pwd_start) {
      pwd_start++;
      char* pwd_end = strchr(pwd_start, '"');
      if (pwd_end) {
        size_t len = pwd_end - pwd_start;
        if (len < sizeof(wifiPassword)) {
          memcpy(wifiPassword, pwd_start, len);
          wifiPassword[len] = '\0';
        }
      }
    }
  }
  
  // Extract name
  char* name_start = strstr(buffer, "\"name\":");
  if (name_start) {
    name_start = strchr(name_start + 7, '"');
    if (name_start) {
      name_start++;
      char* name_end = strchr(name_start, '"');
      if (name_end) {
        size_t len = name_end - name_start;
        if (len < sizeof(deviceName)) {
          memcpy(deviceName, name_start, len);
          deviceName[len] = '\0';
        }
      }
    }
  }
  
  free(buffer);  // Free allocated memory
  
  Serial.println(" OK");
  Serial.print("  SSID: ");
  Serial.println(wifiSSID);
  return true;
}

#endif // WIFI_LOADER_H
