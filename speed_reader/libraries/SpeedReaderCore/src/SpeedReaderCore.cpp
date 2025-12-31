#include "SpeedReaderCore.h"
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <LiquidCrystal.h>

#include "globals.h"
#include "SR_LCDDisplay.h"
#include "SR_Session.h"
#include "SR_SpeedSensor.h"
#include "SR_WiFiLoader.h"
#include "SR_Accelerometer.h"
#include "SR_HTTPHandlers.h"
#include "SR_Tasks.h"
#include "SR_StartupCheck.h"

#if ENABLE_BT
#include <BluetoothSerial.h>
#endif

namespace SpeedReader {

void begin() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nSpeed Reader Starting (Library Mode)...");
  
  // Create mutexes immediately
  dataMutex = xSemaphoreCreateMutex();
  lcdMutex = xSemaphoreCreateMutex();
  
  // Initialize LCD
  Serial.println("Initializing LCD...");
  
  // Pre-initialize LCD pins to known state to prevent garbage
  pinMode(LCD_RS, OUTPUT); digitalWrite(LCD_RS, LOW);
  pinMode(LCD_E, OUTPUT);  digitalWrite(LCD_E, LOW);
  pinMode(LCD_D4, OUTPUT); digitalWrite(LCD_D4, LOW);
  pinMode(LCD_D5, OUTPUT); digitalWrite(LCD_D5, LOW);
  pinMode(LCD_D6, OUTPUT); digitalWrite(LCD_D6, LOW);
  pinMode(LCD_D7, OUTPUT); digitalWrite(LCD_D7, LOW);

  // Set up contrast control via PWM
  pinMode(LCD_CONTRAST, OUTPUT);
  analogWrite(LCD_CONTRAST, LCD_CONTRAST_VALUE);
  Serial.print("LCD contrast set to: ");
  Serial.println(LCD_CONTRAST_VALUE);
  
  delay(100); // 100ms for power stabilization
  lcd.begin(16, 2);
  delay(100);
  updateLCD("Initializing...", "Please wait");
  
  // Load WiFi config
  updateLCD("Loading Config", "SPIFFS...");
  loadWiFiConfig();
  
  // Configure pins
  updateLCD("Configuring", "Pins...");
  pinMode(LED_PIN, OUTPUT);
  pinMode(D4_DIGITAL, INPUT_PULLUP); // Use internal pull-up
  pinMode(D5_ANALOG, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(D5_ANALOG, ADC_11db);
  
  Serial.print("Configured Pins: D4_DIGITAL=");
  Serial.print(D4_DIGITAL);
  Serial.print(" (Speed), D5_ANALOG=");
  Serial.print(D5_ANALOG);
  Serial.println(" (Monitor)");
  
  // Initial read test
  int initialVal = analogRead(D5_ANALOG);
  Serial.print("Initial Analog Read: ");
  Serial.println(initialVal);
  
  // Compute distance per rotation
  // 1 inch = 1/63360 miles
  // Circumference = PI * Diameter
  distancePerRotation_miles = (3.14159265358979323846 * wheelDiameterIn) / 63360.0;
  
  // Adjust for decimal point correction
  distancePerRotation_miles *= 0.1f;
  
  // Apply configuration offset as a calibration adjustment per rotation
  distancePerRotation_miles += distanceOffset;
  
  Serial.print("Dist/Rot (miles): ");
  Serial.println(distancePerRotation_miles, 8);
  
  // Startup blink
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }

  // Initialize Accelerometer
  updateLCD("Accel Init", "Please wait...");
  initAccelerometer();
  
  updateLCD("Accel Calib", "Keep Still...");
  calibrateAccelerometer(50);
  
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
    if (useHTTPS) {
      setupHTTPSServer();
    } else {
      setupHTTPServer();
    }
    
    // Register device
    registerDevice();
    #endif
  } else {
    Serial.println("\nWiFi not available");
    updateLCD("WiFi Failed", "Serial/BT only");
    delay(2000);
  }
  
  // Create FreeRTOS tasks
  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 2048, NULL, 1, &sensorTaskHandle, 0);
  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 2048, NULL, 1, &displayTaskHandle, 1);

  // Run Startup Diagnostics
  runStartupDiagnostics();
  
  // Zero out readings after diagnostics
  resetSession();
  
  // Show ready
  showReady();
  Serial.println("System Ready!");
}

void update() {
  #if ENABLE_HTTP
  if (serverStarted && server) {
    server->loop();
  }
  #endif
  
  delay(10);
}

} // namespace SpeedReader
