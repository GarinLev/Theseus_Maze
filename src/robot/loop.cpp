#include "robot.h"
#include "../soft_move.h"


void robot::loop() {
	wheelA1.updatePI(100);
	node_pi_run(wheelA1.piNode);
	node_motor_run(wheelA1.motorNode);
}
