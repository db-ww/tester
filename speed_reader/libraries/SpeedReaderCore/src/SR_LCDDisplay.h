#ifndef SR_LCD_DISPLAY_H
#define SR_LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal.h>

// LCD Display Functions
void updateLCD(const char* line1, const char* line2);
void showReady();
void showJob(const String& job);
void showSpeed(float speed_mph);

#endif // SR_LCD_DISPLAY_H
