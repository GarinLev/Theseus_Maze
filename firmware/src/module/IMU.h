#ifndef FIRMWARE_IMU_H
#define FIRMWARE_IMU_H

#include <MPU6050_6Axis_MotionApps20.h>
#include "../math/PID.h"

class IMU {
public:
    bool init();
    void update();

    float ypr[3]{};

private:
    PID pid;
    MPU6050 mpu;
    uint8_t fifo_buffer[64] = {};
};

#endif
