#pragma once
#include "../../lib/GyverIO/GyverIO.h"

namespace robot {
    void setGlobalState(int s);
}

struct WheelManager {
    WheelController* fl, * fr, * bl, * br;
    DistNode* wall_sens = nullptr;
    float target_yaw = 0.0f;
    uint8_t pinHitA = 40;
    uint8_t pinHitB = 41;

    struct pt pt_task;

    enum States { IDLE, MOVE_SOFT, HIT, HIT_LEFT, HIT_RIGHT, HIT_BOTH, BLACK };
    States state = IDLE;

    bool wall_disable = false;
    bool is_moving = false;
    uint32_t _timer = 0;

    void init() {
        gio::init(pinHitA, INPUT_PULLUP);
        gio::init(pinHitB, INPUT_PULLUP);
        PT_INIT(&pt_task);
        state = IDLE;
        wall_disable = false;
        Serial.println(F("[WM] Initialized"));
    }

    void setDistSource(DistNode* s) { wall_sens = s; }

    bool getHitBtn() const {
        return gio::read(pinHitA) == LOW || gio::read(pinHitB) == LOW;
    }

    bool isHandlingHit() const {
        return (state != IDLE && state != MOVE_SOFT);
    }

    void startNewTask(float mm, float target_rpm, float* yaw) {
        Serial.println(F("[WM] --- New Task Started ---"));
        PT_INIT(&pt_task);
        moveDistance(mm, target_rpm, yaw, true);
    }

    void moveDistance(float mm, float target_rpm, float* yaw, bool changeState = true) {
        int32_t ticks = (int32_t)WC_MM_TO_TICKS(mm);
        uint32_t acc = (uint32_t)WC_MM_TO_TICKS(abs(0.2f * mm));

        if (changeState) state = MOVE_SOFT;
        is_moving = true;

        fl->goTo(target_rpm, acc, ticks, yaw);
        fr->goTo(target_rpm, acc, ticks, yaw);
        bl->goTo(target_rpm, acc, ticks, yaw);
        br->goTo(target_rpm, acc, ticks, yaw);
    }

    void stop() {
        fl->stop(); fr->stop(); bl->stop(); br->stop();
        state = IDLE;
        is_moving = false;
        wall_disable = false;
    }

    void update() {
        fl->update(); fr->update(); bl->update(); br->update();
        if (wall_sens) node_dist_run(*wall_sens);
        task_process();
    }

    int task_process() {
        PT_BEGIN(&pt_task);
        for (;;) {
            if (state == IDLE) {
                PT_YIELD(&pt_task);
                continue;
            }

            if (state == MOVE_SOFT) {
                PT_WAIT_WHILE(&pt_task, fl->is_moving && !getHitBtn());

                if (!fl->is_moving) {
                    if (wall_sens && wall_sens->dist_valid && wall_sens->dist <= 180) state = HIT;
                    else stop();
                    continue;
                }

                bool L = (gio::read(pinHitA) == LOW);
                bool R = (gio::read(pinHitB) == LOW);
                fl->stop(); fr->stop(); bl->stop(); br->stop();

                if (L && R) state = HIT_BOTH;
                else if (L) state = HIT_LEFT;
                else state = HIT_RIGHT;
                continue;
            }

            if (state == HIT_LEFT || state == HIT_RIGHT || state == HIT_BOTH) {
                Serial.println(F("[WM] Handling Bumper..."));

                moveDistance(-60, 60, nullptr, false);
                _timer = millis();
                PT_WAIT_UNTIL(&pt_task, millis() - _timer > 100);
                PT_WAIT_UNTIL(&pt_task, !fl->is_moving);

                if (state != HIT_BOTH) {
                    float cur_yaw = robot::rotateController.angel.ypr[0];
                    float target = round(cur_yaw / 90.0f) * 90.0f;
                    target_yaw = target;

                    robot::rotateController.run(target - cur_yaw, 55);

                    while (robot::rotateController.is_active) {
                        robot::rotateController.update();
                        fl->update(); fr->update(); bl->update(); br->update();
                        PT_YIELD(&pt_task);
                    }

                    moveDistance(50, 50, &target_yaw, false);
                    PT_WAIT_UNTIL(&pt_task, !fl->is_moving);
                }

                Serial.println(F("[WM] Done."));

                stop();
                continue;
            }

            if (state == HIT) {
                wall_disable = true;
                fl->setSpeed(50); fr->setSpeed(50);
                bl->setSpeed(50); br->setSpeed(50);
                _timer = millis();
                PT_WAIT_UNTIL(&pt_task, getHitBtn() || (millis() - _timer > 2000));
                stop();
                continue;
            }
        }
        PT_END(&pt_task);
    }
};