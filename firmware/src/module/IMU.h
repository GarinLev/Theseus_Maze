#ifndef FIRMWARE_IMU_H
#define FIRMWARE_IMU_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#ifndef DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE
#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE
#endif
#include <DebugLog.h>

class IMU {
public:
    struct YPR {
        float yaw;
        float pitch;
        float roll;
    };

    IMU();

    void init();
    void zero();
    void update();
    YPR get() const;

    float get_nearest_90() const;

    static float snap_to_90(float angle);

    float ypr[3] = {0.0f, 0.0f, 0.0f};

private:
    TwoWire _wire;
    Adafruit_BNO055 _bno;
    YPR _offset;
    YPR _cached;

    static float normalize180(float angle);
};

#endif