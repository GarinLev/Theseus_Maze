#pragma once
#include "controller_wheel.h"
#include "../../lib/GyverIO/GyverIO.h"

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
    enum States { IDLE, MOVE_SOFT, SLOPE_UP, SLOPE_DOWN, HIT };
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
                PT_WAIT_UNTIL(&pt_task, !fl->is_moving || (pitch_ptr && (*pitch_ptr > 15.0f || *pitch_ptr < -15.0f)));
                
                if (pitch_ptr) {
                    if (*pitch_ptr > 15.0f) { state = SLOPE_UP; continue; }
                    if (*pitch_ptr < -15.0f) { state = SLOPE_DOWN; continue; }
                }

                state = HIT;
                continue;
            }

            if (state == SLOPE_UP || state == SLOPE_DOWN) {
                hasDouble = true;
                wall_disable = true;

                if (state == SLOPE_UP) setAllSpeed(120);
                else setAllSpeed(60);

                PT_WAIT_UNTIL(&pt_task, (state == SLOPE_UP ? *pitch_ptr < 5.0f : *pitch_ptr > -5.0f) || state == IDLE);

                if (state != IDLE) {
                    commandAll(_target_v, (uint32_t)WC_MM_TO_TICKS(40), (int32_t)WC_MM_TO_TICKS(150));
                    PT_WAIT_UNTIL(&pt_task, !fl->is_moving || state == IDLE);
                }
                state = HIT;
                continue;
                wall_disable = false;

            }

            if (state == HIT) {
                if (wall_sens && wall_sens->dist_valid && wall_sens->dist <= 120) {

                    if (!getHitBtn()) {
                        setAllSpeed(60);
                        PT_WAIT_UNTIL(&pt_task, getHitBtn() || state == IDLE);
                    }
                    stop();
                    
                    _timer = millis();
                    PT_WAIT_UNTIL(&pt_task, millis() - _timer > 150);

                    PT_WAIT_UNTIL(&pt_task, !fl->is_moving || state == IDLE);
                }
                
                state = IDLE;
                is_moving = false;
            }
        }

        PT_END(&pt_task);
    }
};