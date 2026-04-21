#pragma once

#include "../../lib/pt/pt.h"
#include "../macro.h"
#include "../../lib/MPU6050/MPU6050_6Axis_MotionApps20.h"


struct AngelNode {
    struct pt pt;

    MPU6050 mpu;

    uint32_t dt;
    uint32_t last_time;
    
    uint8_t fifoBuffer[45];

    float ypr[3];
};

void node_angel_init(AngelNode&ctx);
int  node_angel_run(AngelNode&ctx);