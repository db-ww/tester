#include <WiFi.h>
#include <SPIFFS.h>
#include <LiquidCrystal.h>

// Include modular headers
#include "config.h"
#include "globals.h"
#include "lcd_display.h"
#include "session.h"
#include "speed_sensor.h"
#include "wifi_loader.h"
#include "http_handlers.h"
#include "accelerometer.h"
#include "tasks.h"

#if ENABLE_HTTP
#include <WebServer.h>
#endif

#if ENABLE_BT
#include <BluetoothSerial.h>
#endif

// ===== Global Variable Definitions =====
// WiFi credentials
char wifiSSID[64] = "";
char wifiPassword[128] = "";
char deviceName[32] = "speed_reader";

// LCD Object
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Global State
volatile int lastDigital = 0;
volatile int lastAnalog = 0;

// Rotation / speed/distance globals
volatile unsigned long rotationCount = 0;
volatile unsigned long lastRotationMicros = 0;
volatile unsigned long lastRotationIntervalMicros = 0;
volatile unsigned long lastRotationTimeMillis = 0;

float distancePerRotation_m = 0.0f;
float totalDistance_m = 0.0f;
float maxSpeed_m_s = 0.0f;
float currentAngle = 0.0f;

// Job tracking
String currentJob = "";
bool sessionActive = false;

// Radio / server state
#if ENABLE_BT
BluetoothSerial SerialBT;
#endif

bool haveWiFi = false;
bool haveBT = false;
bool serverStarted = false;

#if ENABLE_HTTP
WebServer server(80);
#endif

// Non-blocking timers
unsigned long lastAdcRead = 0;
unsigned long lastDigitalRead = 0;

// FreeRTOS task handles
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;

// Mutex for shared data
SemaphoreHandle_t dataMutex;
SemaphoreHandle_t lcdMutex;

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nSpeed Reader Starting...");
  
  // Create mutexes immediately
  dataMutex = xSemaphoreCreateMutex();
  lcdMutex = xSemaphoreCreateMutex();
  
  // Initialize LCD
  Serial.println("Initializing LCD...");
  
  // Set up contrast control via PWM
  pinMode(LCD_CONTRAST, OUTPUT);
  analogWrite(LCD_CONTRAST, LCD_CONTRAST_VALUE);
  Serial.print("LCD contrast set to: ");
  Serial.println(LCD_CONTRAST_VALUE);
  
  delay(50);
  lcd.begin(16, 2);
  delay(50);
  updateLCD("Initializing...", "Please wait");
  
  // Load WiFi config
  loadWiFiConfig();
  
  // Configure pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(D4_DIGITAL, INPUT_PULLUP); // Use internal pull-up
  pinMode(D5_ANALOG, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(D5_ANALOG, ADC_11db);
  
  // Compute distance per rotation
  distancePerRotation_m = 3.14159265358979323846 * wheelDiameterIn * 0.0254;
  
  // Startup blink
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }

  // Initialize Accelerometer
  initAccelerometer();
  calibrateAccelerometer(50);
  
  // Attach rotation interrupt
  // Attach rotation interrupt
  attachInterrupt(digitalPinToInterrupt(D4_DIGITAL), onRotation, FALLING);
  
  // Start Bluetooth
  #if ENABLE_BT
  Serial.print("Starting Bluetooth as '");
  Serial.print(deviceName);
  Serial.println("'...");
  haveBT = SerialBT.begin(deviceName);
  Serial.println(haveBT ? " OK" : " FAILED");
  #endif
  
  // Start WiFi
  Serial.print("Attempting WiFi connect...");
  updateLCD("Connecting WiFi", wifiSSID);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(deviceName);
  WiFi.begin(wifiSSID, wifiPassword);
  
  unsigned long startAttempt = millis();
  const unsigned long wifiTimeout = 30000UL; // 30 seconds
  
  while (millis() - startAttempt < wifiTimeout) {
    if (WiFi.status() == WL_CONNECTED) {
      haveWiFi = true;
      break;
    }
    delay(500);
    Serial.print('.');
  }
  
  if (!haveWiFi) {
      Serial.println("\nWiFi Connection Failed!");
      Serial.print("SSID: "); Serial.println(wifiSSID);
      Serial.print("Status Code: "); Serial.println(WiFi.status());
      Serial.println("Reasons: 1=No SSID, 4=Fail, 6=Disconnect");
  }
  
  if (haveWiFi) {
    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    char ipStr[20];
    snprintf(ipStr, sizeof(ipStr), "%s", WiFi.localIP().toString().c_str());
    updateLCD("WiFi Connected", ipStr);
    delay(2000);
    
    #if ENABLE_HTTP
    server.on("/", handleRoot);
    server.on("/readings", handleReadings);
    server.on("/start", handleStart);
    server.begin();
    serverStarted = true;
    Serial.println("HTTP server started");
    #endif
  } else {
    Serial.println("\nWiFi not available");
    updateLCD("WiFi Failed", "Serial/BT only");
    delay(2000);
  }
  
  // Create FreeRTOS tasks
  xTaskCreatePinnedToCore(
    sensorTask,
    "SensorTask",
    4096,
    NULL,
    1,
    &sensorTaskHandle,
    0
  );
  
  xTaskCreatePinnedToCore(
    displayTask,
    "DisplayTask",
    4096,
    NULL,
    1,
    &displayTaskHandle,
    1
  );
  
  // Show ready
  showReady();
  Serial.println("System Ready!");
}

// ===== Main Loop =====
void loop() {
  #if ENABLE_HTTP
  if (serverStarted) {
    server.handleClient();
  }
  #endif
  
  delay(10);
}
