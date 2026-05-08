#include <Arduino.h>
#include "controller_debug.h"
#include "../robot/robot.h"

template <size_t N>
void print_nums(uint8_t(&args)[N]) {
    for (size_t i = 0; i < N; i++) {
        Serial.print(args[i]);
        if (i < N - 1) Serial.print(',');
    }
    Serial.println();
}


int DebugController::update()
{
    PT_BEGIN(&pt_task);

    for (;;) {
        timer = millis();
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - timer) >= 250);

        print();
    }

    PT_END(&pt_task);
}

void DebugController::print() {
    node_dist_run(*robot::hitController.sens);
    Serial.println(robot::hitController.sens->dist);
}
