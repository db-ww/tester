#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\startup_check.h"
#ifndef STARTUP_CHECK_H
#define STARTUP_CHECK_H

#include "globals.h"
#include "lcd_display.h"

// Extern declaration for variable defined in accelerometer.h
// (Assuming single translation unit via .ino include)
extern bool isAccelConnected; 

void runStartupDiagnostics() {
    Serial.println("\n\n===== STARTUP DIAGNOSTICS =====");
    
    // Initial user feedback
    updateLCD("Self Test...", "Checking Systems");
    delay(1000);
    
    bool anyFailure = false;
    
    // 1. Accelerometer Check
    Serial.print("[DIAG] Accelerometer: ");
    if (isAccelConnected) {
        Serial.println("OK");
        updateLCD("Sensors:", "Accel OK");
        delay(500);
    } else {
        Serial.println("FAIL - Not Detected");
        updateLCD("Sensors:", "Accel FAILED");
        anyFailure = true;
        delay(2000);
    }
    
    // 2. WiFi Check
    Serial.print("[DIAG] WiFi: ");
    if (haveWiFi) {
        Serial.print("OK - IP: ");
        Serial.println(WiFi.localIP());
        
        char ipStr[17];
        snprintf(ipStr, sizeof(ipStr), "IP %s", WiFi.localIP().toString().c_str());
        updateLCD("WiFi: OK", ipStr);
        delay(1000);
    } else {
        Serial.println("FAIL - Not Connected");
        updateLCD("WiFi: FAILED", "Check Config");
        anyFailure = true;
        delay(2000);
    }
    
    // 3. Bluetooth Check
    #if ENABLE_BT
    Serial.print("[DIAG] Bluetooth: ");
    if (haveBT) {
        Serial.println("OK");
        updateLCD("Bluetooth:", "OK");
        delay(500);
    } else {
        Serial.println("FAIL - Init Error");
        updateLCD("Bluetooth:", "FAILED");
        anyFailure = true;
        delay(2000);
    }
    #endif
    
    // Summary
    Serial.println("===== DIAGNOSTICS COMPLETE =====");
    if (anyFailure) {
        Serial.println("System Status: DEGRADED (Check Logs)");
        updateLCD("System Status:", "Failures Found");
    } else {
        Serial.println("System Status: HEALTHY");
        updateLCD("System Status:", "All Green");
    }
    delay(1500);
}

#endif // STARTUP_CHECK_H
