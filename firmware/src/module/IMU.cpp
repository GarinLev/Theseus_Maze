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

    delay(700);

    return false;
}

void IMU::update() {
    uint16_t fifo_count = mpu.getFIFOCount();
    uint16_t packet_size = mpu.dmpGetFIFOPacketSize();

    if (fifo_count >= 1024) {
        mpu.resetFIFO();
        return;
    }

    if (fifo_count < packet_size) {
        return;
    }

    while (fifo_count >= packet_size) {
        mpu.getFIFOBytes(fifo_buffer, packet_size);
        fifo_count -= packet_size;
    }

    Quaternion q;
    VectorFloat gravity;
    float data_ypr[3];

    mpu.dmpGetQuaternion(&q, fifo_buffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(data_ypr, &q, &gravity);

    ypr[0] = data_ypr[0] * 180.0f / M_PI;
    ypr[1] = data_ypr[1] * 180.0f / M_PI;
    ypr[2] = data_ypr[2] * 180.0f / M_PI;
}