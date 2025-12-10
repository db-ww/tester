#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(23, 22, 5, 18, 19, 21);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n=== LCD 1602 4-bit Test ===");
  Serial.println("Pin mapping:");
  Serial.println("  RS  -> GPIO 23");
  Serial.println("  E   -> GPIO 22");
  Serial.println("  D4  -> GPIO 5");
  Serial.println("  D5  -> GPIO 18");
  Serial.println("  D6  -> GPIO 19");
  Serial.println("  D7  -> GPIO 21");
  Serial.println("\nInitializing LCD...");
  
  // Initialize LCD with delay
  delay(50);
  lcd.begin(16, 2);
  delay(50);
  
  Serial.println("LCD initialized!");
  Serial.println("Writing to LCD...");
  
  // Clear and write
  lcd.clear();
  delay(10);
  lcd.setCursor(0, 0);
  lcd.print("Hello ESP32");
  lcd.setCursor(0, 1);
  lcd.print("LCD 1602 4-bit");
  
  Serial.println("Text written to LCD");
  Serial.println("\nIf LCD is blank but backlight is on:");
  Serial.println("1. Adjust contrast potentiometer (small screw on LCD module)");
  Serial.println("2. Check all pin connections");
  Serial.println("3. Verify LCD is 5V compatible or use level shifters");
}

void loop() {
  // Blink cursor to show it's running
  static unsigned long lastBlink = 0;
  static bool cursorOn = false;
  
  if (millis() - lastBlink > 1000) {
    lastBlink = millis();
    cursorOn = !cursorOn;
    
    if (cursorOn) {
      lcd.cursor();
      Serial.println("Cursor ON");
    } else {
      lcd.noCursor();
      Serial.println("Cursor OFF");
    }
  }
}
