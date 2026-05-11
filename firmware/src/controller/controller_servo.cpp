#include "controller_servo.h"

void ServoController::init() {
    PT_INIT(&pt_task);

    soft.x0 = 0;
    soft.x1 = 0;
    soft.y0 = 0;
    soft.y1 = 0;

    servo.attach(pin);
    servo.write(ServoController_CloseStart);
    set(ServoController_CloseStart);
}

void ServoController::set(float _pos) {
    start_time = millis();
    is_active = true;

    soft.x0 = 0;
    soft.x1 = 1500;
    soft.y0 = (float)position;
    soft.y1 = (float)_pos;
}

int ServoController::update() {
    PT_BEGIN(&pt_task);
    for (;;) {
        timer = millis();
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - timer) >= 50);

        if (is_active) {
            uint32_t elapsed = millis() - start_time;

            float pos = SoftSetGet(&soft, elapsed);
            servo.write((int)pos);

            if (elapsed >= 1500) {
                is_active = false;
                position = soft.y1;
                servo.write((int)soft.y1);
            }
        }
    }
    PT_END(&pt_task);
}