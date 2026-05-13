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
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - timer) >= 20);

        if (is_active) {
            uint32_t elapsed = millis() - start_time;

            if (elapsed >= 2000) {
                servo.write((int)soft.y1);
                position = soft.y1;
                is_active = false;
            }
            else {
                float pos = SoftSetGet(&soft, elapsed);
                servo.write((int)pos);
            }
        }
    }
    PT_END(&pt_task);
}