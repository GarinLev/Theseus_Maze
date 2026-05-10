#include <Arduino.h>
#include "src/robot/robot.h"
void setup() {
    robot::init();
}
void loop() {
    robot::loop();
}
