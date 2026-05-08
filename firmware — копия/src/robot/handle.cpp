#include "robot.h"

void robot::handleWait() {
    wheelManager.stop();
}

void robot::handleMove() {
    node_angel_run(rotateManager.angel);
    wallRight.update();
    wallLeft.update();
    wallManager.update();
    hitController.update();
  }


void robot::handleRotate() {
    rotateManager.update();
}

void robot::handleVictim() {
    servoController.update();
}