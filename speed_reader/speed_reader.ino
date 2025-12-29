#include <WiFi.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <LiquidCrystal.h>

#if ENABLE_HTTP
#include <HTTPServer.hpp>
#include <HTTPSServer.hpp>
#endif

// Include the Core Library
#include <SpeedReaderCore.h>

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

// Config Defaults
float speedOffset = 0.0f;
float distanceOffset = 0.0f;
float angleOffset = 0.0f;
float accelOffset = 0.0f;
float vibrationOffset = 0.0f;
float accelScale = 1.0f;
float speedScale = 1.0f;
int pulsesPerRotation = 1; // Defaulting to 1
char apiKey[64] = "hello"; // Default API Key for initial setup attempt
char devicePassword[32] = "admin"; // Device Password required to change API Key
char registerUrl[128] = ""; 
char station[32] = "DefaultStation";

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
bool useHTTPS = false;  // Default to HTTP to avoid heap issues, can be overridden by config

void setupHTTPServer() {
  Serial.println("Setting up HTTP Server (port 80)...");
  server = new HTTPServer(80);
  
  if (server) {
    registerRoutes(server);
    server->start();
    if (server->isRunning()) {
        serverStarted = true;
        Serial.println("HTTP Server Ready on port 80");
    } else {
        Serial.println("ERROR: HTTP Server failed to start!");
    }
  }
}

void setupHTTPSServer() {
  Serial.printf("Free heap before HTTPS setup: %u bytes\n", ESP.getFreeHeap());
  Serial.println("Setting up HTTPS Server (port 443)...");
  
  // Try to allocate certificate
  cert = new SSLCert();
  if (!cert) {
    Serial.println("ERROR: Failed to allocate SSLCert - out of memory!");
    return;
  }
  
  bool hasCert = false;
  if (SPIFFS.exists("/cert.der") && SPIFFS.exists("/key.der")) {
    Serial.println("Loading cert from SPIFFS...");
    File certFile = SPIFFS.open("/cert.der", "r");
    File keyFile = SPIFFS.open("/key.der", "r");
    
    if (certFile && keyFile) {
       uint16_t certLen = certFile.size();
       uint16_t keyLen = keyFile.size();
       
       Serial.printf("Cert size: %d bytes, Key size: %d bytes\n", certLen, keyLen);
       
       uint8_t * cData = new uint8_t[certLen];
       uint8_t * kData = new uint8_t[keyLen];
       
       if (cData && kData) {
         certFile.read(cData, certLen);
         keyFile.read(kData, keyLen);
         
         *cert = SSLCert(cData, certLen, kData, keyLen);
         hasCert = true;
         Serial.println("Cert loaded from SPIFFS successfully.");
       } else {
         Serial.println("ERROR: Failed to allocate cert buffers!");
         if (cData) delete[] cData;
         if (kData) delete[] kData;
       }
    }
    certFile.close();
    keyFile.close();
  }
  
  if (!hasCert) {
      Serial.println("");
      Serial.println("====== CERTIFICATE REQUIRED ======");
      Serial.println("No certificate found in SPIFFS!");
      Serial.println("");
      Serial.println("This device requires a pre-generated certificate");
      Serial.println("due to memory constraints.");
      Serial.println("");
      Serial.println("Please generate using OpenSSL (see guide)");
      Serial.println("===================================");
      return;
  }
  
  // Create HTTPS server on port 443, limiting to 1 connection to save memory
  server = new HTTPSServer(cert, 443, 1);
  
  if (server) {
    registerRoutes(server);
    server->start();
    if (server->isRunning()) {
        serverStarted = true;
        Serial.println("HTTPS Server Ready on port 443");
    } else {
        Serial.println("ERROR: HTTPS Server failed to start!");
    }
  }
}
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
  // This ensures distance starts at 0 but accumulates with the offset correction
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
    if (useHTTPS) {
      setupHTTPSServer();
    } else {
      setupHTTPServer();
    }
    
    // Register device with remote URL if configured
    if (strlen(registerUrl) > 0) {
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
    }
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
    2048,
    NULL,
    1,
    &sensorTaskHandle,
    0
  );
  
  xTaskCreatePinnedToCore(
    displayTask,
    "DisplayTask",
    2048,
    NULL,
    1,
    &displayTaskHandle,
    1
  );

  // Run Startup Diagnostics
  runStartupDiagnostics();
  
  // Zero out readings after diagnostics (in case sensors picked up noise/movement)
  resetSession();
  
  // Show ready
  showReady();
  Serial.println("System Ready!");
}

// ===== Main Loop =====
void loop() {
  #if ENABLE_HTTP
  if (serverStarted && server) {
    server->loop();
  }
#endif
  
  delay(10);
}
