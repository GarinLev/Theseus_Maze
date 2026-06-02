#ifndef FIRMWARE_STACK_H
#define FIRMWARE_STACK_H

#include <stdint.h>
#include <stddef.h>
#include <new> // Нужно для placement new

template <typename T, uint16_t N, size_t MAX_ITEM_SIZE = 32>
class StaticStack {
    // Выравнивание по типу T гарантирует корректность указателей
    alignas(T) uint8_t buffer[N][MAX_ITEM_SIZE] = {};
    uint16_t count = 0;

public:
    StaticStack() = default;

    // Запрещаем копирование контейнера во избежание проблем с памятью
    StaticStack(const StaticStack&) = delete;
    StaticStack& operator=(const StaticStack&) = delete;

    // Деструктор обязательно должен очистить оставшиеся элементы
    ~StaticStack() {
        while (!isEmpty()) {
            pop();
        }
    }

    template <typename Derived>
    bool push(const Derived& value) {
        static_assert(sizeof(Derived) <= MAX_ITEM_SIZE, "Object size exceeds MAX_ITEM_SIZE");

        if (isFull()) return false;

        // Использование Placement NEW корректно копирует vtable и данные
        new (buffer[count]) Derived(value);

        count++;
        return true;
    }

    bool pop() {
        if (isEmpty()) return false;

        // Явно вызываем деструктор полиморфного интерфейса T
        top().~T();

        --count;
        return true;
    }

    T& top() {
        return *reinterpret_cast<T*>(buffer[count - 1]);
    }

    const T& top() const {
        return *reinterpret_cast<const T*>(buffer[count - 1]);
    }

    bool isEmpty() const { return count == 0; }
    bool isFull() const { return count >= N; }
    uint16_t size() const { return count; }
};

#endif //FIRMWARE_STACK_H
