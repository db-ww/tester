#include <Arduino.h>
#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\speed_reader.ino"
#include "SpeedReaderCore.h"

#line 3 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\speed_reader.ino"
void setup();
#line 7 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\speed_reader.ino"
void loop();
#line 3 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\speed_reader.ino"
void setup() {
  SpeedReader::begin();
}

void loop() {
  SpeedReader::update();
}

