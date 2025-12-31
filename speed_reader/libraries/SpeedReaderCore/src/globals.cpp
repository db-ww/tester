#include "globals.h"
#include <LiquidCrystal.h>

#if ENABLE_HTTP
#include <HTTPServer.hpp>
#include <HTTPSServer.hpp>
#endif

#if ENABLE_BT
#include <BluetoothSerial.h>
#endif

// ===== WiFi credentials =====
char wifiSSID[64] = "";
char wifiPassword[128] = "";
char deviceName[32] = "speed_reader";

// ===== LCD Object =====
// Note: Pins are defined in config.h via globals.h
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// ===== Global State =====
volatile int lastDigital = 0;
volatile int lastAnalog = 0;

// Rotation / speed/distance globals
volatile unsigned long rotationCount = 0;
volatile unsigned long lastRotationMicros = 0;
volatile unsigned long lastRotationIntervalMicros = 0;
volatile unsigned long lastRotationTimeMillis = 0;

float distancePerRotation_miles = 0.0f;
float totalDistance_miles = 0.0f;
float maxSpeed_mph = 0.0f;
float currentAngle = 0.0f;
float maxAngle = -180.0f;
float minAngle = 180.0f;
float currentVibration = 0.0f;
float maxVibration = 0.0f;

// Job tracking
char currentJob[32] = "";
bool sessionActive = false;

// Radio / server state
#if ENABLE_BT
BluetoothSerial SerialBT;
#endif

bool haveWiFi = false;
bool haveBT = false;
bool serverStarted = false;

#if ENABLE_HTTP
HTTPServer *server = NULL;
SSLCert *cert = NULL;
bool useHTTPS = false;
#endif

// Non-blocking timers
unsigned long lastAdcRead = 0;
unsigned long lastDigitalRead = 0;

// FreeRTOS task handles
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;

// Mutex for shared data
SemaphoreHandle_t dataMutex = NULL;
SemaphoreHandle_t lcdMutex = NULL;

// ===== Configurable Offsets =====
float speedOffset = 0.0f;
float distanceOffset = 0.0f;
float angleOffset = 0.0f;
float accelOffset = 0.0f;
float vibrationOffset = 0.0f;
float accelScale = 1.0f;
float speedScale = 1.0f;
int pulsesPerRotation = 1;

// ===== API Key for Authentication =====
char apiKey[64] = "hello";
char devicePassword[32] = "admin";
char registerUrl[128] = "";
char station[32] = "DefaultStation";
