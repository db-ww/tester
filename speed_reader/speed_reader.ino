#include "SpeedReaderCore.h"

void setup() {
  SpeedReader::begin();
}

void loop() {
  SpeedReader::update();
}
