/*
 * HC-SR04 Pin Test - ESP32
 * 
 * This sketch tests if the GPIO pins are working correctly
 * by toggling TRIG and reading ECHO pin states
 */

const int TRIG_PIN = 5;   // GPIO 5
const int ECHO_PIN = 18;  // GPIO 18

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println("\n\n====================================");
  Serial.println("HC-SR04 Pin Test");
  Serial.println("====================================\n");
  
  // Test TRIG pin
  Serial.println("Testing TRIG pin (GPIO 5)...");
  digitalWrite(TRIG_PIN, LOW);
  delay(100);
  Serial.println("  TRIG set to LOW");
  
  digitalWrite(TRIG_PIN, HIGH);
  delay(100);
  Serial.println("  TRIG set to HIGH");
  
  digitalWrite(TRIG_PIN, LOW);
  Serial.println("  TRIG set to LOW");
  Serial.println("  ✓ TRIG pin working\n");
  
  // Test ECHO pin
  Serial.println("Testing ECHO pin (GPIO 18)...");
  Serial.print("  Current ECHO state: ");
  Serial.println(digitalRead(ECHO_PIN) ? "HIGH" : "LOW");
  Serial.println("\n  Try manually connecting ECHO to:");
  Serial.println("  - 3.3V (should read HIGH)");
  Serial.println("  - GND (should read LOW)");
  Serial.println("\n====================================\n");
}

void loop() {
  // Continuously read ECHO pin state
  int echoState = digitalRead(ECHO_PIN);
  
  Serial.print("ECHO: ");
  Serial.print(echoState ? "HIGH (1)" : "LOW  (0)");
  Serial.print(" | ");
  
  // Toggle TRIG for visual feedback
  static bool trigState = false;
  trigState = !trigState;
  digitalWrite(TRIG_PIN, trigState);
  
  Serial.print("TRIG: ");
  Serial.println(trigState ? "HIGH" : "LOW");
  
  delay(500);
}
