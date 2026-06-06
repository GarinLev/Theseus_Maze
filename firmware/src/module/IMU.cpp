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
    int16_t fifoC;
    // This section of code is for when we allowed more than 1 packet to be acquired
    uint32_t BreakTimer = micros();
    uint16_t length = mpu.dmpGetFIFOPacketSize();
    bool packetReceived = false;

    do {
        Serial.print(".");
        if ((fifoC = mpu.getFIFOCount()) > length) {

            if (fifoC > 200) { // if you waited to get the FIFO buffer to > 200 bytes it will take longer to get the last packet in the FIFO Buffer than it will take to  reset the buffer and wait for the next to arrive
                Serial.println("!");
                mpu.resetFIFO(); // Fixes any overflow corruption
                fifoC = 0;
                while (!((fifoC = mpu.getFIFOCount())) && ((micros() - BreakTimer) <= (mpu.getFIFOTimeout()))); // Get Next New Packet
            } else { //We have more than 1 packet but less than 200 bytes of data in the FIFO Buffer
                Serial.println("-");
                uint8_t Trash[I2CDEVLIB_WIRE_BUFFER_LENGTH];
                while ((fifoC = mpu.getFIFOCount()) > length) {  // Test each time just in case the MPU is writing to the FIFO Buffer
                    fifoC = fifoC - length; // Save the last packet
                    uint16_t  RemoveBytes;
                    while (fifoC) { // fifo count will reach zero so this is safe
                        RemoveBytes = (fifoC < I2CDEVLIB_WIRE_BUFFER_LENGTH) ? fifoC : I2CDEVLIB_WIRE_BUFFER_LENGTH; // Buffer Length is different than the packet length this will efficiently clear the buffer
                        mpu.getFIFOBytes(Trash, (uint8_t)RemoveBytes);
                        fifoC -= RemoveBytes;
                    }
                }
            }
        }
        if (!fifoC) return; // Called too early no data or we timed out after FIFO Reset
        // We have 1 packet
        packetReceived = fifoC == length;
        if (!packetReceived && (micros() - BreakTimer) > (mpu.getFIFOTimeout())) return;
    } while (!packetReceived);
    mpu.getFIFOBytes(fifo_buffer, length); //Get 1 packet




    Quaternion q;
    VectorFloat gravity;

    mpu.dmpGetQuaternion(&q, fifo_buffer);
    mpu.dmpGetGravity(&gravity, &q);

    float data_ypr[3];
    mpu.dmpGetYawPitchRoll(data_ypr, &q, &gravity);

    ypr[0] = data_ypr[0] * 180.0f / M_PI;
    ypr[1] = data_ypr[1] * 180.0f / M_PI;
    ypr[2] = data_ypr[2] * 180.0f / M_PI;
}