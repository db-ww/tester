// I2C Accelerometer Reader
// Continuously reads accelerometer data from register 0x00 at address 0x68

#include <Wire.h>

#define I2C_SDA 21  // Default SDA for ESP32
#define I2C_SCL 22  // Default SCL for ESP32
#define ACCEL_ADDR 0x68  // Accelerometer I2C address
#define ACCEL_DATA_REG 0x00  // Data register (big-endian format)

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ACCEL_ADDR);
  Wire.write(reg);
  Wire.write(value);
  uint8_t error = Wire.endTransmission();
  
  Serial.print("Write Reg 0x");
  if (reg < 16) Serial.print("0");
  Serial.print(reg, HEX);
  Serial.print(" = 0x");
  if (value < 16) Serial.print("0");
  Serial.print(value, HEX);
  Serial.print(" -> ");
  Serial.println(error == 0 ? "OK" : "FAILED");
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(ACCEL_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ACCEL_ADDR, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

void initAccelerometer() {
  Serial.println("\nInitializing accelerometer...");
  
  // Read WHO_AM_I or ID register (common locations)
  Serial.println("\nReading ID registers:");
  Serial.print("Reg 0x0F: 0x");
  Serial.println(readRegister(0x0F), HEX);
  Serial.print("Reg 0x75: 0x");
  Serial.println(readRegister(0x75), HEX);
  
  // Try common initialization sequences
  Serial.println("\nTrying initialization sequences:");
  
  // MPU6050 style (0x68 is common MPU address)
  Serial.println("\n1. MPU6050 init:");
  writeRegister(0x6B, 0x00); // PWR_MGMT_1: Wake up, use internal oscillator
  delay(100);
  writeRegister(0x1C, 0x00); // ACCEL_CONFIG: ±2g range
  delay(10);
  
  // ADXL345 style
  Serial.println("\n2. ADXL345 init:");
  writeRegister(0x2D, 0x08); // POWER_CTL: Measurement mode
  delay(10);
  
  // LIS3DH style
  Serial.println("\n3. LIS3DH init:");
  writeRegister(0x20, 0x47); // CTRL_REG1: 50Hz, normal mode, all axes enabled
  delay(10);
  
  Serial.println("\nInitialization complete!\n");
  delay(100);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("I2C Accelerometer Reader");
  Serial.println("=================================");
  Serial.print("Device Address: 0x");
  Serial.println(ACCEL_ADDR, HEX);
  Serial.print("Data Register: 0x");
  Serial.println(ACCEL_DATA_REG, HEX);
  Serial.print("SDA: GPIO");
  Serial.println(I2C_SDA);
  Serial.print("SCL: GPIO");
  Serial.println(I2C_SCL);
  Serial.println("=================================\n");
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // 100kHz standard mode
  
  delay(500);
  
  // Initialize the accelerometer
  initAccelerometer();
}

void readAccelData() {
  Wire.beginTransmission(ACCEL_ADDR);
  Wire.write(0x3B); // MPU6050 accelerometer data register
  uint8_t error = Wire.endTransmission(false);
  
  if (error == 0) {
    Wire.requestFrom(ACCEL_ADDR, (uint8_t)6); // Read 6 bytes (X,Y,Z as 16-bit values)
    
    if (Wire.available() >= 6) {
      // Big-endian: high byte first, then low byte
      int16_t x = (Wire.read() << 8) | Wire.read();
      int16_t y = (Wire.read() << 8) | Wire.read();
      int16_t z = (Wire.read() << 8) | Wire.read();
      
      // Convert to g-force (MPU6050 default range is ±2g)
      // LSB Sensitivity = 16384 LSB/g (from MPU6050 datasheet)
      float x_g = x / 16384.0;
      float y_g = y / 16384.0;
      float z_g = z / 16384.0;
      
      // Display raw values
      Serial.print("Raw: X: ");
      Serial.print(x);
      Serial.print("\tY: ");
      Serial.print(y);
      Serial.print("\tZ: ");
      Serial.print(z);
      
      // Display in g-force
      Serial.print("\t|\tG-force: X: ");
      Serial.print(x_g, 3);
      Serial.print("g\tY: ");
      Serial.print(y_g, 3);
      Serial.print("g\tZ: ");
      Serial.print(z_g, 3);
      Serial.println("g");
    } else {
      Serial.println("Error: Not enough data available");
    }
  } else {
    Serial.print("I2C Error: ");
    Serial.println(error);
  }
}

void loop() {
  readAccelData();
  delay(100); // Read at ~10Hz
}
