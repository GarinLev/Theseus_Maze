#include "controller_rotate.h"
#include "controller_wheel.h"
#include <Arduino.h>

#define PRECISION 4.0f      // Допуск точности в градусах
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
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= UPDATE_INTERVAL);
        last_time = millis();

        if (!is_active) continue;

        float current_yaw = angel.ypr[0];

        // 1. Вычисляем изменение угла за шаг. 
        // normalize_angle здесь КРИТИЧЕСКИ важен: он корректно обработает переход через 180 -> -180
        float delta = normalize_angle(current_yaw - previous_yaw);
        accumulated_yaw += delta;
        previous_yaw = current_yaw;

        // 2. Ошибка — сколько еще нужно докрутить до цели
        float error = target_total_angle - accumulated_yaw;

        // Условие завершения
        if (fabsf(error) < PRECISION) {
            stopWheels();
            is_active = false;
            continue;
        }

        // 3. Для SoftMoveGet передаем абсолютное значение пройденного пути
        float traveled = fabsf(accumulated_yaw);
        float profile_speed = SoftMoveGet(&profile, traveled);

        if (profile_speed < min_start_rpm) profile_speed = min_start_rpm;

        // 4. Направление: если ошибка положительная — крутим в одну сторону, отрицательная — в другую
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
    if (fabsf(total_angle) < 2.5f) { // Игнорируем слишком маленькие углы
        is_active = false;
        stopWheels();
        return;
    }

    // Убираем resetFIFO(), чтобы не терять систему координат
    node_angel_run(angel);

    start_yaw = angel.ypr[0];
    previous_yaw = start_yaw;
    accumulated_yaw = 0; // Сбрасываем только накопитель для конкретного маневра

    target_total_angle = total_angle;

    float abs_angle = fabsf(total_angle);
    profile.x_start = 0;
    profile.L_total = abs_angle;

    // Ограничиваем дистанцию разгона/торможения, чтобы робот не дергался на малых углах
    float acc_dist = abs_angle * 0.4f;
    if (acc_dist > 45.0f) acc_dist = 45.0f; // Максимум 45 градусов на разгон

    profile.d_acc = acc_dist;
    profile.y0 = min_start_rpm;
    profile.y1 = target_v;

    is_active = true;
}

void RotateController::calibrate() {
    node_angel_init(angel);
    node_angel_offsets(angel);
}