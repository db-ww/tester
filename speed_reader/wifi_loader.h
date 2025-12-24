#ifndef WIFI_LOADER_H
#define WIFI_LOADER_H

#include <SPIFFS.h>
#include "globals.h"

// ===== Helper Functions =====
// Simple XOR Cipher with Hex encoding
String encryptDecrypt(String input) {
  char key = 0x5A; // Secret key
  String output = "";
  for (int i = 0; i < input.length(); i++) {
    char c = input[i] ^ key;
    if (c < 0x10) output += "0";
    output += String(c, HEX);
  }
  return output;
}

String decryptHex(String input) {
  char key = 0x5A;
  String output = "";
  for (int i = 0; i < input.length(); i += 2) {
    String byteStr = input.substring(i, i + 2);
    char c = (char)strtol(byteStr.c_str(), NULL, 16);
    output += (char)(c ^ key);
  }
  return output;
}

// Extract JSON value by key
String getJsonValue(String json, String key) {
  int keyIndex = json.indexOf("\"" + key + "\":");
  if (keyIndex == -1) return "";
  
  int colonIndex = json.indexOf(":", keyIndex);
  if (colonIndex == -1) return "";
  
  int valStart = colonIndex + 1;
  
  // Skip whitespace
  while (valStart < json.length() && json[valStart] <= ' ') valStart++;
  
  if (valStart >= json.length()) return "";
  
  // Check if string
  if (json[valStart] == '"') {
      valStart++; // Skip opening quote
      int valEnd = json.indexOf("\"", valStart);
      // Handle simple escaped quote case if needed, but for now standard
      if (valEnd == -1) return ""; 
      return json.substring(valStart, valEnd);
  } else {
      // Number, boolean or null
      int valEnd = valStart;
      // Read until comma, closing brace, or whitespace
      while (valEnd < json.length() && 
             json[valEnd] != ',' && 
             json[valEnd] != '}' && 
             json[valEnd] != ']') {
        valEnd++;
      }
      // Trim trailing whitespace from the token
      String token = json.substring(valStart, valEnd);
      token.trim();
      return token;
  }
}

// ===== WiFi/System Configuration Loader =====
bool loadWiFiConfig() {
  Serial.print("Loading Config from SPIFFS...");
  
  if (!SPIFFS.begin(true)) {
    Serial.println(" FAILED to mount SPIFFS");
    return false;
  }
  
  if (!SPIFFS.exists("/config.json")) {
    Serial.println(" config.json not found, using defaults");
    strncpy(wifiSSID, "BAYSAN", sizeof(wifiSSID) - 1);
    strncpy(wifiPassword, "timetowork", sizeof(wifiPassword) - 1);
    return true;
  }
  
  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println(" Failed to open config.json");
    return false;
  }
  
  String json = file.readString();
  file.close();
  
  // Parse Strings
  String s_ssid = getJsonValue(json, "ssid");
  String s_pass = getJsonValue(json, "password");
  String s_name = getJsonValue(json, "name");
  String s_admin_enc = getJsonValue(json, "admin_pass_enc");
  
  if (s_ssid.length() > 0) strncpy(wifiSSID, s_ssid.c_str(), sizeof(wifiSSID) - 1);
  if (s_pass.length() > 0) strncpy(wifiPassword, s_pass.c_str(), sizeof(wifiPassword) - 1);
  if (s_name.length() > 0) strncpy(deviceName, s_name.c_str(), sizeof(deviceName) - 1);
  
  if (s_admin_enc.length() > 0) {
      String decrypted = decryptHex(s_admin_enc);
      if (decrypted.length() > 0) {
        strncpy(adminPassword, decrypted.c_str(), sizeof(adminPassword) - 1);
      }
  }
  
  // Parse Floats
  String s_speed = getJsonValue(json, "speed_offset");
  String s_dist = getJsonValue(json, "distance_offset");
  String s_speed_scl = getJsonValue(json, "speed_scale");
  String s_pulses = getJsonValue(json, "pulses_per_rotation");
  String s_angle = getJsonValue(json, "angle_offset");
  String s_a_off = getJsonValue(json, "accel_offset");
  String s_a_scl = getJsonValue(json, "accel_scale");
  
  if (s_speed.length() > 0) speedOffset = s_speed.toFloat();
  if (s_speed_scl.length() > 0) speedScale = s_speed_scl.toFloat();
  if (s_pulses.length() > 0) pulsesPerRotation = s_pulses.toInt();
  if (s_dist.length() > 0) distanceOffset = s_dist.toFloat();
  if (s_angle.length() > 0) angleOffset = s_angle.toFloat();
  if (s_a_off.length() > 0) accelOffset = s_a_off.toFloat();
  if (s_a_scl.length() > 0) accelScale = s_a_scl.toFloat();
  
  Serial.println(" OK");
  Serial.print("  SSID: "); Serial.println(wifiSSID);
  Serial.print("  Offsets: S="); Serial.print(speedOffset);
  Serial.print(" D="); Serial.print(distanceOffset);
  Serial.print(" A="); Serial.println(angleOffset);
  
  return true;
}

void saveConfig() {
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
      Serial.println("Failed to open config.json for writing");
      return;
  }
  
  String encPass = encryptDecrypt(String(adminPassword));
  
  String json = "{";
  json += "\"ssid\":\"" + String(wifiSSID) + "\",";
  json += "\"password\":\"" + String(wifiPassword) + "\",";
  json += "\"name\":\"" + String(deviceName) + "\",";
  json += "\"admin_pass_enc\":\"" + encPass + "\",";
  json += "\"speed_offset\":" + String(speedOffset) + ",";
  json += "\"speed_scale\":" + String(speedScale) + ",";
  json += "\"pulses_per_rotation\":" + String(pulsesPerRotation) + ",";
  json += "\"distance_offset\":" + String(distanceOffset) + ",";
  json += "\"angle_offset\":" + String(angleOffset) + ",";
  json += "\"accel_offset\":" + String(accelOffset) + ",";
  json += "\"accel_scale\":" + String(accelScale);
  json += "}";
  
  file.print(json);
  file.close();
  Serial.println("Config saved.");
}

#endif // WIFI_LOADER_H
