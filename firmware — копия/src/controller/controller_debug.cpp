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
    static int8_t prev_task = -1;
    static int8_t prev_state = -1;
    static int8_t prev_sub = -1;

    int8_t t = robot::task;
    int8_t s = robot::state;
    int8_t sub = robot::current_sub_step;

    if (t == prev_task && s == prev_state && sub == prev_sub) return;

    prev_task = t;
    prev_state = s;
    prev_sub = sub;

    Serial.print("Task:");
    Serial.print(t);
    Serial.print("\tState:");
    Serial.print(s);
    Serial.print("\tSub:");
    Serial.println(sub);
}
