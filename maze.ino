#include <Arduino.h>
#include "src/robot/robot.h"
void setup() {
    Serial.begin(115200);
    while (! Serial) { delay(1); }
    robot::init();
}
void loop() {
    robot::loop();
}
