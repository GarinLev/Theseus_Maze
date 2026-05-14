#include "node_angel.h"

void node_angel_init(AngelNode& ctx) {
    PT_INIT(&ctx.pt);

    ctx.mpu.initialize();
    if (!ctx.mpu.testConnection()) {
        Serial.println(F("MPU6050 connection failed"));
        return;
    }

    uint8_t devStatus = ctx.mpu.dmpInitialize();
    if (devStatus == 0) {
        ctx.mpu.setXAccelOffset(-2242);
        ctx.mpu.setYAccelOffset(-3245);
        ctx.mpu.setZAccelOffset(2878);
        ctx.mpu.setXGyroOffset(-1127);
        ctx.mpu.setYGyroOffset(-39);
        ctx.mpu.setZGyroOffset(-11);

        ctx.mpu.setDMPEnabled(true);
        ctx.mpu.resetFIFO();
    }
    else {
        Serial.print(F("DMP Error: "));
        Serial.println(devStatus);
    }
}

int node_angel_run(AngelNode& ctx) {
    if (ctx.mpu.dmpGetCurrentFIFOPacket(ctx.fifoBuffer)) {
        Quaternion q;
        VectorFloat gravity;

        ctx.mpu.dmpGetQuaternion(&q, ctx.fifoBuffer);
        ctx.mpu.dmpGetGravity(&gravity, &q);
        ctx.mpu.dmpGetYawPitchRoll(ctx.ypr, &q, &gravity);

        ctx.ypr[0] = ctx.ypr[0] * 180.0f / M_PI;
        ctx.ypr[1] = -ctx.ypr[1] * 180.0f / M_PI;
        ctx.ypr[2] = ctx.ypr[2] * 180.0f / M_PI;
        return 1;
    }
    return 0;
}

void node_angel_offsets(AngelNode& ctx) {
    ctx.mpu.setXAccelOffset(0);
    ctx.mpu.setYAccelOffset(0);
    ctx.mpu.setZAccelOffset(0);
    ctx.mpu.setXGyroOffset(0);
    ctx.mpu.setYGyroOffset(0);
    ctx.mpu.setZGyroOffset(0);

    ctx.mpu.CalibrateAccel(10);
    ctx.mpu.CalibrateGyro(10);

    Serial.print('\n');
    int16_t* offsets = ctx.mpu.GetActiveOffsets();
    for (int i = 0; i < 6; i++) {
        Serial.print(offsets[i]);
        if (i < 5) Serial.print(F(", "));
    }
    Serial.println();
}