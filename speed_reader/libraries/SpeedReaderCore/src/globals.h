#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include "config.h"

// FreeRTOS includes for ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

// Forward declarations to avoid heavy includes here
class LiquidCrystal;
#if ENABLE_HTTP
namespace httpsserver {
  class HTTPServer;
  class SSLCert;
}
using namespace httpsserver;
#endif

#if ENABLE_BT
class BluetoothSerial;
#endif

// ===== WiFi credentials loaded at runtime from SPIFFS =====
extern char wifiSSID[64];
extern char wifiPassword[128];
extern char deviceName[32];

// ===== LCD Object =====
extern LiquidCrystal lcd;

// ===== Global State =====
extern volatile int lastDigital;
extern volatile int lastAnalog;

// Rotation / speed/distance globals
extern volatile unsigned long rotationCount;
extern volatile unsigned long lastRotationMicros;
extern volatile unsigned long lastRotationIntervalMicros;
extern volatile unsigned long lastRotationTimeMillis;

extern float distancePerRotation_miles;
extern float totalDistance_miles;
extern float maxSpeed_mph;
extern float currentAngle;
extern float maxAngle;
extern float minAngle;

// Job tracking
extern char currentJob[32];
extern bool sessionActive;

// Radio / server state
#if ENABLE_BT
extern BluetoothSerial SerialBT;
#endif

extern bool haveWiFi;
extern bool haveBT;
extern bool serverStarted;

#if ENABLE_HTTP
extern HTTPServer *server; // Base class for both servers (HTTPServer or HTTPSServer)
extern SSLCert *cert;
extern bool useHTTPS;  // Control HTTP vs HTTPS
#endif

// Non-blocking timers
extern unsigned long lastAdcRead;
extern unsigned long lastDigitalRead;

// FreeRTOS task handles
extern TaskHandle_t sensorTaskHandle;
extern TaskHandle_t displayTaskHandle;

// Mutex for shared data
extern SemaphoreHandle_t dataMutex;
extern SemaphoreHandle_t lcdMutex;

// ===== Configurable Offsets =====
extern float speedOffset;
extern float distanceOffset;
extern float angleOffset;
extern float accelOffset;
extern float accelScale;
extern float speedScale;
extern int pulsesPerRotation;

extern float currentVibration;
extern float maxVibration;
extern float vibrationOffset;

// ===== API Key for Authentication =====
extern char apiKey[64];
extern char devicePassword[32];
extern char registerUrl[128];
extern char station[32];

#endif // GLOBALS_H
