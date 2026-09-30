#ifndef TASK_STACK_H
#define TASK_STACK_H

#include <stdint.h>
#include <stddef.h>
#include "task/Task.h"

// Placement new для AVR (не требует <new>)
inline void* operator new(size_t, void* ptr) noexcept { return ptr; }

template <size_t MaxTasks, size_t BytesPerTaskEstimate = 64>
class TaskArenaStack {
public:
    static constexpr size_t BufferSize = MaxTasks * BytesPerTaskEstimate;
    static constexpr size_t max_tasks = MaxTasks;

private:
    // Буфер памяти под разнородные задачи
    alignas(2) uint8_t memory[BufferSize];
    Task* tasks[MaxTasks];
    uint16_t offsets[MaxTasks];

    size_t count = 0;
    size_t current_offset = 0;

public:
    TaskArenaStack() = default;
    TaskArenaStack(const TaskArenaStack&) = delete;
    TaskArenaStack& operator=(const TaskArenaStack&) = delete;

    ~TaskArenaStack() {
        clear();
    }

    // Поддержка вызова push(TaskDelay(700)) или push(rot)
    template <typename ConcreteTask>
    bool push(const ConcreteTask& task) {
        if (count >= MaxTasks) return false;

        // Выравнивание по 2 байта (для работы с парами регистров на AVR)
        size_t aligned_offset = (current_offset + 1) & ~((size_t)1);
        size_t next_offset = aligned_offset + sizeof(ConcreteTask);
        if (next_offset > BufferSize) return false;

        void* place = (void*)(memory + aligned_offset);
        Task* new_task = new (place) ConcreteTask(task);

        offsets[count] = current_offset; // точка возврата памяти для pop()
        tasks[count] = new_task;
        count++;
        current_offset = next_offset;
        return true;
    }

    // Экономичный пуш без копирования: emplace<TaskDelay>(700)
    template <typename ConcreteTask, typename... Args>
    bool emplace(Args... args) {
        if (count >= MaxTasks) return false;

        size_t aligned_offset = (current_offset + 1) & ~((size_t)1);
        size_t next_offset = aligned_offset + sizeof(ConcreteTask);
        if (next_offset > BufferSize) return false;

        void* place = (void*)(memory + aligned_offset);
        Task* new_task = new (place) ConcreteTask(args...);

        offsets[count] = current_offset;
        tasks[count] = new_task;
        count++;
        current_offset = next_offset;
        return true;
    }

    // Удаление верхней задачи с возвратом байтов в буфер (LIFO)
    void pop() {
        if (count == 0) return;
        count--;
        tasks[count]->~Task();
        current_offset = offsets[count];
    }

    Task* top() {
        return (count > 0) ? tasks[count - 1] : nullptr;
    }

    void clear() {
        while (count > 0) {
            pop();
        }
        current_offset = 0;
    }

    // Совместимость с кодом Robot.cpp и Link.cpp
    bool isEmpty() const { return count == 0; }
    bool empty() const { return count == 0; }
    size_t size() const { return count; }

    // Свободное место в буфере (в байтах) с учётом выравнивания следующего push
    size_t free_bytes() const {
        const size_t aligned_offset = (current_offset + 1) & ~((size_t)1);
        return BufferSize - aligned_offset;
    }

    // Занятое место в буфере (в байтах)
    size_t used_bytes() const { return current_offset; }

    Task* operator[](size_t index) {
        return (index < count) ? tasks[index] : nullptr;
    }
};

#endif // TASK_STACK_H