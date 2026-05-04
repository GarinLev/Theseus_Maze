#pragma once

#include "../node/nodes.h"
#include "../../lib/pt/pt.h"
#include "../../lib/GyverIO/GyverIO.h"

struct WallController {
    DistNode nodeUp, nodeWall;
    PI_Node wallNode;
    PI_Topic wallTopic;

    struct pt pt_task;
    uint32_t last_time = 0;

    int16_t correction = 0;
    bool correction_valid = false;

    void init() {
        wallNode.topic = &wallTopic;
        wallNode.Kp = 1.0f;
        wallNode.Ki = 0.000f;
        wallNode.dt = 25;
        wallNode.setpoint = 0;
        PT_INIT(&pt_task);
    }

    void setPins(TwoWire* wire, uint8_t addrUp, uint8_t addrWall,
        uint8_t pinShtUp, uint8_t pinShtWall) {
        nodeUp.wire = wire;
        nodeUp.addr = addrUp;
        nodeUp.pin_sht = pinShtUp;

        nodeWall.wire = wire;
        nodeWall.addr = addrWall;
        nodeWall.pin_sht = pinShtWall;

        node_dist_init(nodeUp);
        node_dist_init(nodeWall);
    }

    void reset() {
        node_dist_reset(nodeUp);
        node_dist_reset(nodeWall);
    }

    void setAddr() {
        auto setupNode = [](DistNode& node) {
                node_dist_unreset(node);
                delay(30);
                node.lox.setBus(node.wire);
                node.lox.setTimeout(500);
                if (!node.lox.init()) {
                    Serial.print(F("Error init by pin "));
                    Serial.println(node.pin_sht);
                    return;
                }
                node.lox.setAddress(node.addr);
                delay(10);
                node.lox.startContinuous(25);
                delay(50);
            };
        setupNode(nodeUp);
        setupNode(nodeWall);
    }

    static constexpr float COS45 = 0.70710678118;

    static constexpr float X1 = 55.0;
    static constexpr float X2 = 55.0;
    static constexpr float Y2 = 63.0;

        
    int task_process() {
        PT_BEGIN(&pt_task);
        for (;;) {
            PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
            last_time = millis();

            if (nodeUp.dist_valid && nodeWall.dist_valid) {

                float d1 = (float)nodeWall.dist;
                float d2 = (float)nodeUp.dist;

                if (d1 > 200 || d2 > 300) {
                    correction_valid = false;
                }
                else {
                    float x1 = X1 + d1;
                    float x2 = X2 + d2 * COS45;
                    float y2 = Y2 + d2 * COS45;

                    float angle_error_deg = atan2(x2 - x1, y2) * 180.0f / PI;

                    if (abs(angle_error_deg) < 90.0f) {
                        wallTopic.value = angle_error_deg;
                        NOTIFY_TOPIC(&wallTopic);
                        correction = wallNode.value_out;
                        correction_valid = true;
                    }
                    else {
                        correction_valid = false;
                    }
                }
            }
            else {
                correction_valid = false;
            }

            if (!correction_valid) {
                wallNode.integral = 0;
                correction = 0;
            }
        }
        PT_END(&pt_task);
    }



    void update() {
        node_dist_run(nodeUp);
        node_dist_run(nodeWall);
        task_process();
        node_pi_run(wallNode);
    }
};