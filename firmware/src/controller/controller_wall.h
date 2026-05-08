#pragma once

#include "../node/nodes.h"
#include "../../lib/pt/pt.h"
#include "../../lib/GyverIO/GyverIO.h"

struct WallController {
    DistNode nodeUp, nodeWall, nodeExtra;
    PI_Node wallNode;
    PI_Topic wallTopic;

    struct pt pt_task;
    uint32_t last_time = 0;

    int16_t correction = 0;
    bool correction_valid = false;


    static constexpr float COS45 = 0.70710678118;

    static constexpr float X1 = 55.0;
    static constexpr float X2 = 55.0;
    static constexpr float Y2 = 63.0;

    void init();
    void setPins(TwoWire* wire, uint8_t addrUp, uint8_t addrWall, uint8_t addrExtra,
        uint8_t pinShtUp, uint8_t pinShtWall, uint8_t pinExtra);
    void reset();
    void setAddr();
    int task_process();
    void update();

};