#ifndef SR_SESSION_H
#define SR_SESSION_H

#include <Arduino.h>

// Session Management Functions
void resetSession();
void startSession(const String& job);
void endSession();

#endif // SR_SESSION_H
