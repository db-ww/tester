#ifndef SR_SPEED_SENSOR_H
#define SR_SPEED_SENSOR_H

#include <Arduino.h>

// Speed Sensor Functions
void IRAM_ATTR onRotation();
float getCurrentSpeed();

#endif // SR_SPEED_SENSOR_H
