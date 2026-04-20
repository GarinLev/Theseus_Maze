#include "robot.h"
#include "../soft_move.h"


void robot::loop() {
    wheelA1.update();
    wheelA2.update();
    wheelB1.update();
    wheelB2.update();
    
}
