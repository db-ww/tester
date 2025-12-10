#define DIGITAL_PIN 34
#define ANALOG_PIN 32

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to wake up
  
  pinMode(DIGITAL_PIN, INPUT);
  pinMode(ANALOG_PIN, INPUT);
  
  // Configure ADC - use 12-bit (native resolution for ESP32)
  analogReadResolution(12);                     // 12-bit -> 0..4095
  analogSetPinAttenuation(ANALOG_PIN, ADC_11db); // 0-3.3V range
  
  Serial.println("\n\nHall Sensor Test Starting...");
  Serial.println("Reading Digital Pin 34 and Analog Pin 32");
  Serial.println("Note: GPIO35 is input-only, no pull-up/down available");
  Serial.println("If reading 4095, the pin may be floating (not connected)");
  Serial.println("Expected range: 0-4095 (12-bit ADC)");
  Serial.println("Voltage = (reading / 4095) * 3.3V");
  Serial.println("");
}

void loop() {
  int dVal = digitalRead(DIGITAL_PIN);
  int aVal = analogRead(ANALOG_PIN);
  float voltage = (aVal / 4095.0) * 3.3;

  Serial.print("Digital(");
  Serial.print(DIGITAL_PIN);
  Serial.print("): ");
  Serial.print(dVal);
  Serial.print(" \tAnalog(");
  Serial.print(ANALOG_PIN);
  Serial.print("): ");
  Serial.print(aVal);
  Serial.print(" (");
  Serial.print(voltage, 2);
  Serial.println("V)");

  delay(100); // 10Hz update rate
}
