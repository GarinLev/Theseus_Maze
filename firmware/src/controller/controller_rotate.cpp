#include "controller_rotate.h"
#include "controller_wheel.h"
#include <Arduino.h>

#define PRECISION 1.5f      // Допуск точности в градусах
#define UPDATE_INTERVAL 20  // Интервал обновления (мс)

void RotateController::init() {
    PT_INIT(&pt_task);
    node_angel_init(angel);
}

float RotateController::normalize_angle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

int RotateController::update() {
    node_angel_run(angel);

    PT_BEGIN(&pt_task);
    for (;;) {
        // Ждем интервала времени
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= UPDATE_INTERVAL);
        last_time = millis();

        if (!is_active) continue;

        float current_yaw = angel.ypr[0];
        // Вычисляем, на сколько мы реально повернулись от старта
        float relative_yaw = normalize_angle(current_yaw - start_yaw);
        // Ошибка: сколько осталось до цели
        float error = normalize_angle(target_total_angle - relative_yaw);

        // Условие завершения движения
        if (fabsf(error) < PRECISION) {
            stopWheels();
            is_active = false;
            continue;
        }

        // Расчет пройденного пути для профиля скорости
        float traveled = fabsf(target_total_angle) - fabsf(error);
        float profile_speed = SoftMoveGet(&profile, max(0.0f, traveled));

        // Ограничиваем минимальную скорость, чтобы не застрять из-за трения
        if (profile_speed < min_start_rpm) profile_speed = min_start_rpm;

        // Определяем направление вращения
        float direction = (error > 0) ? 1.0f : -1.0f;
        applySpeed(profile_speed * direction);
    }
    PT_END(&pt_task);
}

void RotateController::applySpeed(float speed) {
    if (!wheelA1) return;
    // Сбрасываем смещения (если они использовались для движения по прямой)
    wheelA1->speed_offset = 0; wheelA2->speed_offset = 0;
    wheelB1->speed_offset = 0; wheelB2->speed_offset = 0;

    // Танковый разворот: левые и правые борта в разные стороны
    wheelA1->setSpeed(speed); wheelA2->setSpeed(speed);
    wheelB1->setSpeed(-speed); wheelB2->setSpeed(-speed);
}

void RotateController::stopWheels() {
    if (!wheelA1) return;
    wheelA1->stop(); wheelA2->stop();
    wheelB1->stop(); wheelB2->stop();
}

void RotateController::run(float total_angle, float target_v) {
    if (total_angle == 0) {
        is_active = false;
        stopWheels();
        return;
    }

    // Критически важно: сбрасываем FIFO, чтобы не было старых данных
    angel.mpu.resetFIFO();
    delay(10); 

    node_angel_run(angel); // Читаем актуальный угол
    start_yaw = angel.ypr[0];
    target_total_angle = total_angle;

    float abs_angle = fabsf(total_angle);
    profile.x_start = 0;
    profile.L_total = abs_angle;
    profile.d_acc = abs_angle * 0.3f; // 30% пути на разгон
    profile.d_acc = abs_angle * 0.3f; // 30% пути на торможение
    profile.y0 = min_start_rpm;
    profile.y1 = target_v;

    is_active = true;
}

void RotateController::calibrate() {
    node_angel_init(angel);
    node_angel_offsets(angel);
}