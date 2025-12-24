#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "config.h"

#if ENABLE_HTTP
#include <WebServer.h>
#endif

#if ENABLE_BT
#include <BluetoothSerial.h>
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
extern WebServer server;
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

// ===== Admin Password =====
extern char adminPassword[32];

#endif // GLOBALS_H
