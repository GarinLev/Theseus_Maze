#pragma once
#include "controller_wheel.h"
#include "../../lib/GyverIO/GyverIO.h"
#include "../controller/controller_color.h"
#include "../controller/controller_rotate.h"

namespace robot {
    extern ColorController colorController;
    extern RotateController rotateController;
    extern WallController wallRight;
    extern uint16_t aaaa;

}

struct WheelManager {
    WheelController* fl;
    WheelController* fr;
    WheelController* bl;
    WheelController* br;
    float* pitch_ptr = nullptr;
    float* yaw_ptr = nullptr;
    DistNode* wall_sens = nullptr;

    uint8_t pinHitA = 40;
    uint8_t pinHitB = 41;

    struct pt pt_task;
    enum States { IDLE, MOVE_SOFT, HIT, HIT_LEFT, HIT_RIGHT, HIT_BOTH, BLACK };
    States state = IDLE;

    float _target_v;
    uint32_t _accel_ticks;
    int32_t _total_ticks;
    bool is_moving = false;
    bool wall_disable = false;
    uint32_t _timer = 0;
    bool hasDouble = false;

    void init() {
        gio::init(pinHitA, INPUT_PULLUP);
        gio::init(pinHitB, INPUT_PULLUP);
        PT_INIT(&pt_task);
        state = IDLE;
    }

    void setPitchSource(float* p) { pitch_ptr = p; }
    void setDistSource(DistNode* s) { wall_sens = s; }

    bool getHitBtn() const {
        return gio::read(pinHitA) == LOW && gio::read(pinHitB) == LOW;
    }

    void commandAll(float v, uint32_t acc, int32_t ticks) {
        fl->goTo(v, acc, ticks, yaw_ptr);
        fr->goTo(v, acc, ticks, yaw_ptr);
        bl->goTo(v, acc, ticks, yaw_ptr);
        br->goTo(v, acc, ticks, yaw_ptr);
    }

    void setAllSpeed(int16_t rpm) {
        fl->setSpeed(rpm); fr->setSpeed(rpm);
        bl->setSpeed(rpm); br->setSpeed(rpm);
    }

    void moveDistance(float mm, float target_rpm, float* yaw, float accel_mm = -1) {
        if (accel_mm == -1) accel_mm = 0.4f * abs(mm);
        _total_ticks = (int32_t)WC_MM_TO_TICKS(mm);
        _accel_ticks = (uint32_t)WC_MM_TO_TICKS(abs(accel_mm));
        _target_v = target_rpm;
        yaw_ptr = yaw;

        commandAll(_target_v, _accel_ticks, _total_ticks);
        state = MOVE_SOFT;
        is_moving = true;
        PT_INIT(&pt_task);
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
                PT_WAIT_WHILE(&pt_task, fl->is_moving && !(
                    (getHitBtn()) ||
                    ((gio::read(pinHitA) == LOW || gio::read(pinHitB) == LOW) && robot::wallRight.nodeExtra.dist > 20)
                    ));

                // if (robot::colorController.isBlack())
                // {
                //     robot::colorController.update();
                //     state = BLACK;
                //     continue;
                // }

                if (gio::read(pinHitA) == HIGH && gio::read(pinHitB) == LOW)
                {
                    state = HIT_LEFT;
                    continue;
                }

                if (gio::read(pinHitA) == LOW && gio::read(pinHitB) == LOW)
                {
                    state = HIT_BOTH;
                    continue;
                }

                if (gio::read(pinHitA) == LOW && gio::read(pinHitB) == HIGH)
                {
                    state = HIT_RIGHT;
                    continue;
                }



                state = HIT;
                continue;
            }
            // --- УДАР СЛЕВА ---
            if (state == HIT_LEFT) {
                // 1. Чуть отъезжаем назад, чтобы освободить место для маневра
                moveDistance(-40, 60, nullptr);
                PT_WAIT_UNTIL(&pt_task, !fl->is_moving);

                // 3. Пытаемся проехать "шаг" вперед (150мм), чтобы выйти из зоны столкновения
                moveDistance(150, 70, yaw_ptr);
                state = MOVE_SOFT;
                // Не используем continue, чтобы сразу перейти к PT_WAIT_WHILE в начале MOVE_SOFT
            }

            // --- УДАР СПРАВА ---
            if (state == HIT_RIGHT) {
                moveDistance(-40, 60, nullptr);
                PT_WAIT_UNTIL(&pt_task, !fl->is_moving);

                

                moveDistance(150, 70, yaw_ptr);
                state = MOVE_SOFT;
            }

            // --- УДАР ОБОИМИ КОНЦЕВИКАМИ ---
            if (state == HIT_BOTH) {
                // Сильный откат назад
                moveDistance(-80, 80, nullptr);
                PT_WAIT_UNTIL(&pt_task, !fl->is_moving);

                _timer = millis();
                PT_WAIT_UNTIL(&pt_task, millis() - _timer > 200);

                state = MOVE_SOFT;
            }
            if (state == BLACK)
            {
                wall_disable = true;
                robot::aaaa = 0;
                moveDistance(-100, 50, nullptr);
                PT_WAIT_UNTIL(&pt_task, !fl->is_moving);
                stop();
                state = IDLE;
                wall_disable = false;
            }
            if (state == HIT) {
                wall_disable = true;
                if (wall_sens && wall_sens->dist_valid && wall_sens->dist <= 200) {
                    if (!getHitBtn()) {
                        setAllSpeed(40);
                        _timer = millis();

                        PT_WAIT_UNTIL(&pt_task, getHitBtn() || (millis() - _timer > 2000) || state == IDLE);
                    }

                    stop(); 

                    _timer = millis();
                    PT_WAIT_UNTIL(&pt_task, millis() - _timer > 150);
                    wall_disable = false;

                }
                state = IDLE;
                is_moving = false;
                wall_disable = false; 
            }
        }
        PT_END(&pt_task);
    }
};