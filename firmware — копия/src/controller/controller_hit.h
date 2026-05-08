#pragma once
#include "../../lib/pt/pt.h"
#include "../../lib/GyverIO/GyverIO.h"

struct HitController {
    struct pt pt_task;
    uint8_t pinA, pinB;
    DistNode* sens;
    WheelManager* wheelManager;
    bool is_active = false;
    uint16_t timer = 0;

    void init() {
        gio::init(pinA, INPUT_PULLUP);
        gio::init(pinB, INPUT_PULLUP);
        PT_INIT(&pt_task);
    }

    void setPins(uint8_t a, uint8_t b, WheelManager* wm) {
        pinA = a; pinB = b;
        wheelManager = wm;
    }

    bool getBtn() {
        return gio::read(pinA) == LOW && gio::read(pinB) == LOW;
    }

    int update() {
        node_dist_run(*sens);

        PT_BEGIN(&pt_task);
        for (;;) {
            PT_WAIT_UNTIL(&pt_task, is_active);

            wheelManager->fr->setSpeed(70);
            wheelManager->fl->setSpeed(70);
            wheelManager->br->setSpeed(70);
            wheelManager->bl->setSpeed(70);

            PT_WAIT_UNTIL(&pt_task, getBtn());

            wheelManager->stop();


            timer = millis();
            PT_WAIT_UNTIL(&pt_task, millis() - timer > 100);

            wheelManager->moveDistance(-50, 75, nullptr);


            PT_WAIT_UNTIL(&pt_task, !wheelManager->isMoving());

            is_active = false;
        }
        PT_END(&pt_task);
    }
};
