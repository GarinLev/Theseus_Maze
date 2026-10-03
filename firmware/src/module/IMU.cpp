#include "IMU.h"
#include <math.h>
#include "Robot.h"

IMU::IMU()
    : _wire(Wire),
      _bno(-1, 0x28, &_wire),
      _offset{0.0f, 0.0f, 0.0f},
      _cached{0.0f, 0.0f, 0.0f} {}

void IMU::init() {
    if (!_bno.begin(OPERATION_MODE_IMUPLUS)) {
        LOG_ERROR("BNO055 not detected");
        Robot::instance().error_status |= 1;
        return;
    }

    // Даем процессору BNO055 время сойтись по гравитации (акселерометру)
    delay(400);

    // Первоначальный сброс смещения
    zero();
}

void IMU::zero() {
    sensors_event_t event;
    _bno.getEvent(&event);

    // Сохраняем сырые показания ориентации датчика, а не _cached!
    _offset.yaw   = event.orientation.x;
    _offset.pitch = event.orientation.z;
    _offset.roll  = event.orientation.y;

    _cached.yaw   = 0.0f;
    _cached.pitch = 0.0f;
    _cached.roll  = 0.0f;

    ypr[0] = 0.0f;
    ypr[1] = 0.0f;
    ypr[2] = 0.0f;
}

void IMU::update() {
    sensors_event_t event;
    _bno.getEvent(&event);

    _cached.yaw   = normalize180(event.orientation.x - _offset.yaw);
    _cached.pitch = normalize180(event.orientation.z - _offset.pitch);
    _cached.roll  = normalize180(event.orientation.y - _offset.roll);

    ypr[0] = _cached.yaw;
    ypr[1] = _cached.pitch;
    ypr[2] = _cached.roll;

    LOG_INFO(ypr[0], ypr[1], ypr[2]);
}

IMU::YPR IMU::get() const {
    return _cached;
}

float IMU::snap_to_90(float angle) {
    // Приводим угол к положительному диапазону [0, 360)
    while (angle < 0.0f)   angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;

    // Округляем к ближайшему кратному 90
    float snapped = roundf(angle / 90.0f) * 90.0f;

    // 360° эквивалентно 0°
    if (snapped >= 360.0f) {
        snapped = 0.0f;
    }
    return snapped;
}

float IMU::get_nearest_90() const {
    return snap_to_90(_cached.yaw);
}

float IMU::normalize180(float angle) {
    while (angle > 180.0f)  angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}