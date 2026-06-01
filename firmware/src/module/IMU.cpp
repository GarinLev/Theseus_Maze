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
        mpu.setXAccelOffset(-2224);
        mpu.setYAccelOffset(-3280);
        mpu.setZAccelOffset(1132);
        mpu.setXGyroOffset(-1854);
        mpu.setYGyroOffset(-69);
        mpu.setZGyroOffset(-17);

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

        float data_ypr[3];
        mpu.dmpGetYawPitchRoll(data_ypr, &q, &gravity);

        ypr[0] = data_ypr[0] * 180.0f / M_PI; // Yaw
        ypr[1] = data_ypr[1] * 180.0f / M_PI; // Pitch
        ypr[2] = data_ypr[2] * 180.0f / M_PI; // Roll

        return true;
    }
    return false;
}