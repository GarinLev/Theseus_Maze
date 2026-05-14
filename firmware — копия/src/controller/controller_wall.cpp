#include "controller_wall.h"

void WallController::init() {
    wallNode.topic = &wallTopic;
    wallNode.Kp = 0.4f;
    wallNode.Ki = 0.01f;
    wallNode.dt = 25;
    wallNode.setpoint = 0;
    wallNode.integral = 0;
    PT_INIT(&pt_task);
}

void WallController::setPins(TwoWire* wire, uint8_t addrUp, uint8_t addrWall, uint8_t addrExtra,
    uint8_t pinShtUp, uint8_t pinShtWall, uint8_t pinExtra) {
    nodeUp.wire = wire;
    nodeUp.addr = addrUp;
    nodeUp.pin_sht = pinShtUp;

    nodeWall.wire = wire;
    nodeWall.addr = addrWall;
    nodeWall.pin_sht = pinShtWall;

    nodeExtra.wire = wire;
    nodeExtra.addr = addrExtra;
    nodeExtra.pin_sht = pinExtra;

    node_dist_init(nodeUp);
    node_dist_init(nodeWall);
    node_dist_init(nodeExtra);
}

void WallController::reset() {
    node_dist_reset(nodeUp);
    node_dist_reset(nodeWall);
    node_dist_reset(nodeExtra);
}

void WallController::setAddr() {
    auto setupNode = [](DistNode& node) {
        node_dist_unreset(node);
        delay(20);

        node.lox.setBus(node.wire);
        node.lox.setTimeout(500);

        node.lox.setAddress(node.addr);
        delay(10);

        if (!node.lox.init()) {
            Serial.print(F("Error init at 0x"));
            Serial.print(node.addr, HEX);
            Serial.print(F(" pin "));
            Serial.println(node.pin_sht);
            return;
        }

        node.lox.startContinuous(25);
        delay(10);
        };

    setupNode(nodeUp);
    setupNode(nodeWall);
    setupNode(nodeExtra);
}

int WallController::task_process() {
    PT_BEGIN(&pt_task);
    for (;;) {
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
        last_time = millis();

        if (nodeUp.dist_valid && nodeWall.dist_valid && nodeUp.dist > 5 && nodeWall.dist > 5) {
            float d1 = (float)nodeWall.dist;
            float d2 = (float)nodeUp.dist;

            if (d1 > 200 || d2 > 200) {
                correction_valid = false;
            }
            else {
                float x1 = X1 + d1;
                float x2 = X2 + d2 * COS45;
                float y2 = Y2 + d2 * COS45;

                float angle_error_deg = atan2(x2 - x1, y2) * 180.0f / PI;

                if (abs(angle_error_deg) < 90.0f) {
                    wallTopic.value = angle_error_deg;

                    float error = wallNode.setpoint - wallTopic.value;
                    wallNode.integral += error * (wallNode.dt / 1000.0f);
                    wallNode.integral = constrain(wallNode.integral, -255.0f, 255.0f);

                    wallNode.value_out = (error * wallNode.Kp) + (wallNode.integral * wallNode.Ki);

                    correction = (int16_t)wallNode.value_out;
                    correction_valid = true;

                    NOTIFY_TOPIC(&wallTopic);
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

void WallController::update() {
    node_dist_run(nodeUp);
    node_dist_run(nodeWall);
    task_process();
}