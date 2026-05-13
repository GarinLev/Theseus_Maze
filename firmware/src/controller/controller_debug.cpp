#include <Arduino.h>
#include "controller_debug.h"
#include "../robot/robot.h"

template <size_t N>
void print_nums(uint8_t(&args)[N]) {
    for (size_t i = 0; i < N; i++) {
        Serial.print(args[i]);
        if (i < N - 1) Serial.print(',');
    }
    Serial.println();
}


int DebugController::update()
{
    PT_BEGIN(&pt_task);

    for (;;) {
        timer = millis();
        PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - timer) >= 250);

        print();
    }

    PT_END(&pt_task);
}

void DebugController::print() {
    // Храним предыдущие значения, чтобы сравнивать их
    static robot::TaskRobot last_task = robot::TaskRobot_WAIT;
    static robot::StateRobot last_state = robot::StateRobot_WAIT;
    static uint16_t last_step_ls = 0;
    static uint16_t last_vict_ls = 0;

    // Проверяем, изменилось ли хоть что-то
    bool changed = (robot::task != last_task) ||
        (robot::state != last_state) ||
        (robot::step_local_state != last_step_ls) ||
        (robot::victim_local_state != last_vict_ls);

    if (changed) {
        Serial.print(F("[LOG] "));

        // Основная задача
        Serial.print(F("Task:")); Serial.print(robot::task);

        // Текущее состояние (MOVE, ROTATE и т.д.)
        Serial.print(F(" | St:")); Serial.print(robot::state);

        // Локальные шаги автоматов
        Serial.print(F(" | StepL:")); Serial.print(robot::step_local_state);
        Serial.print(F(" | VictL:")); Serial.print(robot::victim_local_state);

        // Если есть сохраненная задача (прерывание)
        if (robot::saved_task != robot::TaskRobot_WAIT) {
            Serial.print(F(" | SAVED:")); Serial.print(robot::saved_task);
        }

        // Состояние моторов
        Serial.print(F(" | Motor:"));
        Serial.print(robot::wheelManager.is_moving ? F("RUN") : F("IDLE"));

        Serial.println(); // Перенос строки

        // Обновляем "последние" значения
        last_task = robot::task;
        last_state = robot::state;
        last_step_ls = robot::step_local_state;
        last_vict_ls = robot::victim_local_state;
    }
}