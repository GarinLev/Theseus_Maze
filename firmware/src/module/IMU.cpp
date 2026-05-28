#include "IMU.h"
#include "Log.h"

bool IMU::init() {
    pid.reset();

    if (!mpu.testConnection()) {
        LOG_ERROR("MPU6050 connection failed!");
        return false;
    }

    mpu.initialize();

    uint8_t dev_status = mpu.dmpInitialize();

    if (dev_status == 0) {
        mpu.setXAccelOffset(-2286);
        mpu.setYAccelOffset(-3251);
        mpu.setZAccelOffset(1132);
        mpu.setXGyroOffset(-1864);
        mpu.setYGyroOffset(-69);
        mpu.setZGyroOffset(-18);

        mpu.setDMPEnabled(true);
        mpu.resetFIFO();

        return true;
    }

    if (dev_status != 0)
    {
        LOG_ERROR("Initialization failed. Code: ", dev_status);
        return false;
    }

    return false;
}

bool IMU::get(float *ypr) {
    if (mpu.dmpGetCurrentFIFOPacket(fifo_buffer)) {
        Quaternion q;
        VectorFloat gravity;

        mpu.dmpGetQuaternion(&q, fifo_buffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

        ypr[0] = ypr[0] * 180.0f / M_PI;
        ypr[1] = ypr[1] * 180.0f / M_PI;
        ypr[2] = ypr[2] * 180.0f / M_PI;

        return true;
    }

    return false;
}
