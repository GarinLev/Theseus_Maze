#include "robot.h"

void robot::loop() {
    debug.frame();
    debug.update();
    debug.ups_update();

    state_update();

    wheelManager.update();

    wallRight.update();
    wallLeft.update();

    wallManager.update();
}
int robot::state_update() {
    static uint32_t wait_tm = 0;
    static int i;
    PT_BEGIN(&task);

    for (;;) {
        wheelManager.moveDistance(310, 85);
        PT_WAIT_WHILE(&task, wheelManager.isMoving());


        wait_tm = millis();
        PT_WAIT_WHILE(&task, millis() - wait_tm < 1000);

        wheelManager.moveDistance(310, 85);
        PT_WAIT_WHILE(&task, wheelManager.isMoving());

        wait_tm = millis();
        PT_WAIT_WHILE(&task, millis() - wait_tm < 1000);

        wheelManager.moveDistance(310, 85);
        PT_WAIT_WHILE(&task, wheelManager.isMoving());

        wait_tm = millis();
        PT_WAIT_WHILE(&task, millis() - wait_tm < 1000);

        wheelManager.moveDistance(310, 85);
        PT_WAIT_WHILE(&task, wheelManager.isMoving());





        PT_WAIT_WHILE(&task, true);

    }

    PT_END(&task);
}
