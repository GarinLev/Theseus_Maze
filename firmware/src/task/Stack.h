#ifndef TASK_STACK_H
#define TASK_STACK_H

#include <new>
#include <stdint.h>
#include <stddef.h>

#include <etl/largest.h>
#include <etl/type_traits.h>
#include <etl/utility.h>

#include "Task.h"

template <size_t MAX_TASKS, typename... TaskTypes>
class TaskArenaStack {
    enum : size_t {
        ITEM_SIZE  = etl::largest<TaskTypes...>::size,
        ITEM_ALIGN = etl::largest<TaskTypes...>::alignment,
        ARENA_SIZE = MAX_TASKS * (ITEM_SIZE + ITEM_ALIGN - 1),
    };

    alignas(ITEM_ALIGN) uint8_t arena[ARENA_SIZE];
    size_t arena_offset = 0;

    Task* stack[MAX_TASKS];
    size_t task_count = 0;

public:
    TaskArenaStack() = default;

    TaskArenaStack(const TaskArenaStack&) = delete;
    TaskArenaStack& operator=(const TaskArenaStack&) = delete;

    ~TaskArenaStack() {
        clear();
    }

    template <typename T>
    bool push(T&& task_obj) {
        if (task_count >= MAX_TASKS) return false;

        using UnqualifiedT = typename etl::remove_cv<typename etl::remove_reference<T>::type>::type;
        static_assert(sizeof(UnqualifiedT) <= ITEM_SIZE, "Task type is too large for this stack");

        constexpr size_t align = alignof(UnqualifiedT);
        size_t current_addr = reinterpret_cast<size_t>(&arena[arena_offset]);
        size_t aligned_addr = (current_addr + align - 1) & ~(align - 1);
        size_t padding = aligned_addr - current_addr;

        if (arena_offset + padding + sizeof(UnqualifiedT) > ARENA_SIZE) {
            return false;
        }

        arena_offset += padding;
        Task* created = new (&arena[arena_offset]) UnqualifiedT(etl::forward<T>(task_obj));
        arena_offset += sizeof(UnqualifiedT);

        stack[task_count++] = created;
        return true;
    }

    void pop() {
        if (isEmpty()) return;

        Task* top_task = stack[task_count - 1];

        top_task->~Task();
        arena_offset = reinterpret_cast<uint8_t*>(top_task) - arena;

        --task_count;
    }

    void clear() {
        while (!isEmpty()) {
            pop();
        }
        arena_offset = 0;
    }

    Task* top() {
        return isEmpty() ? nullptr : stack[task_count - 1];
    }

    const Task* top() const {
        return isEmpty() ? nullptr : stack[task_count - 1];
    }

    bool isEmpty() const { return task_count == 0; }
    bool isFull() const { return task_count >= MAX_TASKS; }
    size_t size() const { return task_count; }
};

#endif // TASK_STACK_H