// I2C Accelerometer Scanner and Test
// Scans I2C bus for devices and attempts to read accelerometer data

#include <Wire.h>

#define I2C_SDA 21  // Default SDA for ESP32
#define I2C_SCL 22  // Default SCL for ESP32
#define ACCEL_INT_PIN 18 // Interrupt from accelerometer

// Common accelerometer I2C addresses
const uint8_t COMMON_ADDRESSES[] = {
  0x18, 0x19, // LIS3DH, LIS2DH
  0x1D, 0x1E, // ADXL345, MMA8451
  0x53,       // ADXL345 (alt)
  0x68, 0x69, // MPU6050, MPU9250
  0x6A, 0x6B  // LSM6DS3, ICM20948
};

volatile bool interruptTriggered = false;
uint8_t foundAddress = 0;

void IRAM_ATTR accelInterruptISR() {
  interruptTriggered = true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("I2C Accelerometer Scanner");
  Serial.println("=================================");
  Serial.print("SDA: GPIO");
  Serial.println(I2C_SDA);
  Serial.print("SCL: GPIO");
  Serial.println(I2C_SCL);
  Serial.print("INT: GPIO");
  Serial.println(ACCEL_INT_PIN);
  Serial.println("");
  
  // Setup interrupt pin
  pinMode(ACCEL_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ACCEL_INT_PIN), accelInterruptISR, RISING);
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // 100kHz standard mode
  
  Serial.println("Scanning I2C bus...");
  scanI2C();
  
  if (foundAddress != 0) {
    Serial.println("\n=================================");
    Serial.println("Attempting to read from device...");
    Serial.println("=================================");
  }
}

void scanI2C() {
  int deviceCount = 0;
  
  Serial.println("\nScanning addresses 0x00 to 0x7F...");
  
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
      deviceCount++;
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // Check if it's a known accelerometer address
      bool isKnown = false;
      for (int i = 0; i < sizeof(COMMON_ADDRESSES); i++) {
        if (address == COMMON_ADDRESSES[i]) {
          isKnown = true;
          if (foundAddress == 0) foundAddress = address;
          break;
        }
      }
      
      if (isKnown) {
        Serial.print(" <- Possible accelerometer!");
      }
      Serial.println();
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("\nNo I2C devices found!");
    Serial.println("\nTroubleshooting:");
    Serial.println("  1. Check SDA/SCL connections");
    Serial.println("  2. Verify accelerometer is powered");
    Serial.println("  3. Check for pull-up resistors (usually 4.7k to 3.3V)");
    Serial.println("  4. Try different I2C pins if your board supports it");
  } else {
    Serial.print("\nFound ");
    Serial.print(deviceCount);
    Serial.println(" device(s)");
  }
}

void readAccelRegisters(uint8_t address) {
  Serial.println("\nReading first 16 registers:");
  
  for (uint8_t reg = 0; reg < 16; reg++) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    uint8_t error = Wire.endTransmission(false);
    
    if (error == 0) {
      Wire.requestFrom(address, (uint8_t)1);
      if (Wire.available()) {
        uint8_t value = Wire.read();
        Serial.print("Reg 0x");
        if (reg < 16) Serial.print("0");
        Serial.print(reg, HEX);
        Serial.print(": 0x");
        if (value < 16) Serial.print("0");
        Serial.print(value, HEX);
        Serial.print(" (");
        Serial.print(value);
        Serial.println(")");
      }
    }
  }
}

void tryReadAccelData(uint8_t address) {
  // Try common register addresses for accelerometer data
  // Most accelerometers store X,Y,Z data in consecutive registers
  
  const uint8_t dataRegisters[] = {
    0x28, // LIS3DH, LIS2DH (OUT_X_L)
    0x32, // ADXL345 (DATAX0)
    0x00, // Some sensors start at 0x00
    0x3B, // MPU6050 (ACCEL_XOUT_H)
  };
  
  for (int i = 0; i < sizeof(dataRegisters); i++) {
    uint8_t startReg = dataRegisters[i];
    
    Serial.print("\nTrying data register 0x");
    if (startReg < 16) Serial.print("0");
    Serial.print(startReg, HEX);
    Serial.println(":");
    
    Wire.beginTransmission(address);
    Wire.write(startReg);
    uint8_t error = Wire.endTransmission(false);
    
    if (error == 0) {
      Wire.requestFrom(address, (uint8_t)6); // Read 6 bytes (X,Y,Z as 16-bit values)
      
      if (Wire.available() >= 6) {
        int16_t x = Wire.read() | (Wire.read() << 8);
        int16_t y = Wire.read() | (Wire.read() << 8);
        int16_t z = Wire.read() | (Wire.read() << 8);
        
        Serial.print("  X: ");
        Serial.print(x);
        Serial.print("\tY: ");
        Serial.print(y);
        Serial.print("\tZ: ");
        Serial.println(z);
        
        // Check if values look reasonable (not all zeros or all 0xFF)
        if ((x != 0 || y != 0 || z != 0) && 
            (x != -1 || y != -1 || z != -1)) {
          Serial.println("  ^ This looks like valid accelerometer data!");
        }
      }
    }
  }
}

void loop() {
  // Check for interrupt
  if (interruptTriggered) {
    interruptTriggered = false;
    Serial.println("\n[INT] Accelerometer interrupt triggered!");
  }
  
  if (foundAddress != 0) {
    Serial.println("\n---------------------------------");
    Serial.print("Reading from device at 0x");
    if (foundAddress < 16) Serial.print("0");
    Serial.println(foundAddress, HEX);
    Serial.println("---------------------------------");
    
    readAccelRegisters(foundAddress);
    tryReadAccelData(foundAddress);
    
    delay(2000);
  } else {
    // Re-scan periodically if no device found
    delay(5000);
    Serial.println("\nRe-scanning...");
    scanI2C();
  }
}
