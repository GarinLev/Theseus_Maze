#include <Arduino.h>
#include "controller_light.h"
#include "../robot/robot.h"

void LightController::init()
{
    PT_INIT(&pt_task);
    strip.setBrightness(255);
    strip.clear();
    strip.show();
}

void LightController::on()
{
    timer = millis();
    isEnable = true;
    strip.fill(color);

    strip.show();
}

void LightController::off()
{
    strip.clear();
    strip.show();
}

int LightController::update()
{
    PT_BEGIN(&pt_task);

    for (;;) {
        PT_WAIT_UNTIL(&pt_task, isEnable && (uint32_t)(millis() - timer) >= LightController_Time);

        if (isEnable) {
            off();
            isEnable = false;
        }
    }

    PT_END(&pt_task);
}