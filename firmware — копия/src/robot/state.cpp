#include "robot.h"

void robot::loop() {
    debug.frame();
    debug.update();
    debug.ups_update();

    state_update();
    (stateTable[state])();

    wheelManager.update();  
}
int robot::state_update() {
    PT_BEGIN(&thread);

    for (;;) {
        if (task == TaskRobot_STEP_UP) {
            state = StateRobot_MOVE;



            PT_WAIT_WHILE(&thread, wheelManager.isMoving());
            node_dist_run(*hitController.sens);
            if (hitController.sens->dist_valid && hitController.sens->dist <= 130) {
                hitController.is_active = true;
                PT_WAIT_WHILE(&thread, hitController.is_active);
            }
           

            task = TaskRobot_WAIT;
        }

        else if (task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_RIGHT
            || task == TaskRobot_VICTIM_RIGHT_X2 || task == TaskRobot_VICTIM_LEFT_X2) {

            state = StateRobot_VICTIM;

            // --- ПЕРВЫЙ ВЫБРОС ---
            if (task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2)
                servoController.set(ServoController_Left);
            else if (task == TaskRobot_VICTIM_RIGHT || task == TaskRobot_VICTIM_RIGHT_X2)
                servoController.set(ServoController_Right);

            PT_WAIT_WHILE(&thread, servoController.is_active);

            if (task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2)
                servoController.set(ServoController_CloseLeft);
            else if (task == TaskRobot_VICTIM_RIGHT || task == TaskRobot_VICTIM_RIGHT_X2)
                servoController.set(ServoController_CloseRight);

            PT_WAIT_WHILE(&thread, servoController.is_active);


            // --- ВТОРОЙ ВЫБРОС (только если X2) ---
            if (task == TaskRobot_VICTIM_LEFT_X2 || task == TaskRobot_VICTIM_RIGHT_X2) {

                if (task == TaskRobot_VICTIM_LEFT_X2)
                    servoController.set(ServoController_Left);
                else if (task == TaskRobot_VICTIM_RIGHT_X2)
                    servoController.set(ServoController_Right);

                PT_WAIT_WHILE(&thread, servoController.is_active);

                if (task == TaskRobot_VICTIM_LEFT_X2)
                    servoController.set(ServoController_CloseLeft);
                else if (task == TaskRobot_VICTIM_RIGHT_X2)
                    servoController.set(ServoController_CloseRight);

                PT_WAIT_WHILE(&thread, servoController.is_active);
            }

            task = TaskRobot_WAIT;
        }


        else if (task == TaskRobot_WAIT) {
            state = StateRobot_WAIT;
        }

        PT_YIELD(&thread); 
    }

    PT_END(&thread);
}