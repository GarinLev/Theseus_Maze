#include "Task.h"
#include <Arduino.h>

void TaskDelay::execute() {
    // 1. Инициализация при первом вызове задачи
    if (!started) {
        start_time = millis();
        started = true;
        state = StateTask::RUNNING;
    }

    // 2. Проверка, истекло ли время
    if (millis() - start_time >= delay_ms) {
        state = StateTask::CLOSE; // Завершаем задачу
    } else {
        state = StateTask::RUNNING;
        
        // Опционально: если у вас в этот класс передается указатель на Robot* (как в TaskMove),
        // можно явно сбрасывать скорость на время паузы:
        // if (robot != nullptr) {
        //     robot->rpm = 0;
        //     robot->steer = 0;
        // }
    }
}