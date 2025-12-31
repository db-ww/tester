#include "SR_WiFiLoader.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <HTTPClient.h>
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
  String s_pass = getJsonValue(json, "wifi_password");
  String s_name = getJsonValue(json, "name");
  String s_admin_enc = getJsonValue(json, "admin_pass_enc");
  String s_api_key = getJsonValue(json, "api_key");
  String s_dev_pass = getJsonValue(json, "device_password");
  String s_reg_url = getJsonValue(json, "register_url");
  String s_station = getJsonValue(json, "station");

  // Handle WIFI password with encryption fallback
  String s_pass_enc = getJsonValue(json, "wifi_password_enc");
  if (s_pass_enc.length() > 0) {
      Serial.println("  Loading encrypted WiFi password...");
      String dec = decryptHex(s_pass_enc);
      strncpy(wifiPassword, dec.c_str(), sizeof(wifiPassword) - 1);
  } else if (s_pass.length() > 0) {
      Serial.println("  Loading plain WiFi password...");
      strncpy(wifiPassword, s_pass.c_str(), sizeof(wifiPassword) - 1);
  } else {
      Serial.println("  WARNING: No WiFi password found in config!");
  }

  // Handle Device password with encryption fallback
  String s_dev_pass_enc = getJsonValue(json, "device_password_enc");
  if (s_dev_pass_enc.length() > 0) {
      String dec = decryptHex(s_dev_pass_enc);
      strncpy(devicePassword, dec.c_str(), sizeof(devicePassword) - 1);
  } else if (s_dev_pass.length() > 0) {
      strncpy(devicePassword, s_dev_pass.c_str(), sizeof(devicePassword) - 1);
  }

  // Handle API Key with encryption fallback
  String s_api_enc = getJsonValue(json, "api_key_enc");
  if (s_api_enc.length() > 0) {
      String dec = decryptHex(s_api_enc);
      strncpy(apiKey, dec.c_str(), sizeof(apiKey) - 1);
  } else if (s_api_key.length() > 0) {
      strncpy(apiKey, s_api_key.c_str(), sizeof(apiKey) - 1);
  } else if (s_admin_enc.length() > 0) {
      String dec = decryptHex(s_admin_enc);
      strncpy(apiKey, dec.c_str(), sizeof(apiKey) - 1);
  }

  if (s_ssid.length() > 0) strncpy(wifiSSID, s_ssid.c_str(), sizeof(wifiSSID) - 1);
  if (s_name.length() > 0) strncpy(deviceName, s_name.c_str(), sizeof(deviceName) - 1);
  if (s_reg_url.length() > 0) strncpy(registerUrl, s_reg_url.c_str(), sizeof(registerUrl) - 1);
  if (s_station.length() > 0) strncpy(station, s_station.c_str(), sizeof(station) - 1);
  
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

  // Parse HTTPS flag
  #if ENABLE_HTTP
  String s_https = getJsonValue(json, "use_https");
  if (s_https.length() > 0) {
    useHTTPS = (s_https == "true" || s_https == "1");
  }
  #endif
  
  Serial.println(" OK");
  Serial.print("  SSID: "); Serial.println(wifiSSID);
  Serial.print("  Offsets: S="); Serial.print(speedOffset);
  Serial.print(" D="); Serial.print(distanceOffset);
  Serial.print(" A="); Serial.println(angleOffset);
  #if ENABLE_HTTP
  Serial.print("  HTTPS Mode: "); Serial.println(useHTTPS ? "ENABLED" : "DISABLED (HTTP)");
  #endif
  
  return true;
}

void saveConfig() {
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
      Serial.println("Failed to open config.json for writing");
      return;
  }
  
  String json = "{";
  json += "\"ssid\":\"" + String(wifiSSID) + "\",";
  json += "\"wifi_password_enc\":\"" + encryptDecrypt(String(wifiPassword)) + "\",";
  json += "\"name\":\"" + String(deviceName) + "\",";
  json += "\"device_password_enc\":\"" + encryptDecrypt(String(devicePassword)) + "\",";
  json += "\"api_key_enc\":\"" + encryptDecrypt(String(apiKey)) + "\",";
  json += "\"register_url\":\"" + String(registerUrl) + "\",";
  json += "\"station\":\"" + String(station) + "\",";
  json += "\"use_https\":" + String(useHTTPS ? "true" : "false") + ",";
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

void registerDevice() {
  if (strlen(registerUrl) > 0 && WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0,0,0,0)) {
    Serial.printf("Registering device: %s at station %s with %s\n", deviceName, station, registerUrl);
    HTTPClient http;
    http.begin(registerUrl);
    http.addHeader("Content-Type", "application/json");

    String regJson = "{";
    regJson += "\"name\":\"" + String(deviceName) + "\",";
    regJson += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    regJson += "\"station\":\"" + String(station) + "\"";
    regJson += "}";

    int httpCode = http.POST(regJson);
    if (httpCode > 0) {
      Serial.printf("Registration successful, response: %d\n", httpCode);
    } else {
      Serial.printf("Registration failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else if (strlen(registerUrl) > 0) {
    Serial.println("Skipping registration - WiFi not fully ready");
  }
}
