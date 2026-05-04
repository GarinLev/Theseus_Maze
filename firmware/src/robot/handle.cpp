#include "robot.h"

void robot::handleWait() {
    wheelManager.stop();
}

void robot::handleMove() {
    wallRight.update();
    wallLeft.update();
    wallManager.update();
}

void robot::handleRotate() {
    rotateManager.update();
}