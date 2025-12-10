/*
 * HC-SR04 Ultrasonic Distance Sensor - ESP32
 * 
 * Connections:
 * VCC -> 5V (or 3.3V)
 * GND -> GND
 * TRIG -> GPIO 5
 * ECHO -> GPIO 18
 * 
 * This sketch reads distance measurements from the HC-SR04 sensor
 * and displays them on the Serial Monitor.
 * 
 * Note: ESP32 GPIO pins are 3.3V tolerant. The HC-SR04 ECHO pin outputs 5V,
 * so consider using a voltage divider or level shifter for the ECHO pin
 * to protect the ESP32, or use a 3.3V compatible HC-SR04 module.
 */

// Pin definitions (ESP32 GPIO pins)
const int TRIG_PIN = 5;   // GPIO 5
const int ECHO_PIN = 18;  //  GPIO 18

// Variables for distance calculation
long duration;
float distance_cm;
float distance_inch;

void setup() {
  // Initialize serial communication (ESP32 standard baud rate)
  Serial.begin(115200);
  
  // Small delay for serial to initialize
  delay(100);
  
  // Configure pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Ensure trigger starts LOW
  digitalWrite(TRIG_PIN, LOW);
  
  // Initial message
  Serial.println("\n\n====================================");
  Serial.println("HC-SR04 Ultrasonic Distance Sensor");
  Serial.println("ESP32 Debug Version");
  Serial.println("====================================");
  Serial.print("TRIG Pin: GPIO ");
  Serial.println(TRIG_PIN);
  Serial.print("ECHO Pin: GPIO ");
  Serial.println(ECHO_PIN);
  Serial.println("====================================\n");
  
  delay(1000);
}

void loop() {
  // Clear the trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  
  // Send a 10 microsecond pulse to trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read the echo pin with timeout (30ms = ~5m max distance)
  // pulseIn() returns the duration (in microseconds) of the pulse
  // Returns 0 if timeout occurs
  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  // Debug: Show raw pulse duration
  Serial.print("Raw pulse: ");
  Serial.print(duration);
  Serial.print(" μs | ");
  
  // Check if we got a valid reading
  if (duration == 0) {
    Serial.println("ERROR: No echo received (timeout)");
    Serial.println("  -> Check wiring:");
    Serial.println("     - TRIG connected to GPIO 5?");
    Serial.println("     - ECHO connected to GPIO 18?");
    Serial.println("     - VCC connected to 5V or 3.3V?");
    Serial.println("     - GND connected?");
    Serial.println("     - Sensor powered on?");
    Serial.println();
    delay(1000);
    return;
  }
  
  // Calculate distance
  // Speed of sound is 343 m/s or 0.0343 cm/microsecond
  // Distance = (Time × Speed of Sound) / 2
  // Divide by 2 because the sound travels to the object and back
  distance_cm = duration * 0.0343 / 2;
  distance_inch = distance_cm / 2.54;
  
  // Display results on Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance_cm, 2);  // 2 decimal places
  Serial.print(" cm  |  ");
  Serial.print(distance_inch, 2);
  Serial.print(" inches");
  
  // Check for out of range readings
  if (distance_cm > 400) {
    Serial.println("  -> Too far (max 400 cm)");
  } else if (distance_cm < 2) {
    Serial.println("  -> Too close (min 2 cm)");
  } else {
    Serial.println("  ✓ Valid reading");
  }
  
  // Wait before next measurement
  delay(500);
}
