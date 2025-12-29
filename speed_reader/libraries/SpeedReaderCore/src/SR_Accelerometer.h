#ifndef SR_ACCELEROMETER_H
#define SR_ACCELEROMETER_H

#include <Arduino.h>

void initAccelerometer();
void updateAngle();
void calibrateAccelerometer(int samples = 200);

extern bool isAccelConnected;
extern float rawAngle;
extern int16_t debug_raw_x, debug_raw_y, debug_raw_z;

#endif // SR_ACCELEROMETER_H
