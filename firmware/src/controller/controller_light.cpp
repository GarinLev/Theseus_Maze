#include <Arduino.h>
#include "controller_light.h"
#include "../robot/robot.h"

void LightController::init()
{
    PT_INIT(&pt_task);
    strip.clear();
    strip.setBrightness(100);
    strip.show();
}

void LightController::on()
{
    timer = millis();

    if (color == RED) strip.fill(mRed);
    else if (color == GREEN) strip.fill(mGreen);
    else if (color == BLUE) strip.fill(mBlue);
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
