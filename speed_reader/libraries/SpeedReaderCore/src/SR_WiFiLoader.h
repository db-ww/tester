#ifndef SR_WIFI_LOADER_H
#define SR_WIFI_LOADER_H

#include <Arduino.h>

// Helper Functions
String encryptDecrypt(String input);
String decryptHex(String input);
String getJsonValue(String json, String key);

// WiFi/System Configuration Loader
bool loadWiFiConfig();
void saveConfig();
void registerDevice();

#endif // SR_WIFI_LOADER_H
