#ifndef FIRMWARE_STACK_H
#define FIRMWARE_STACK_H

#include <new>

template <typename T, uint16_t N, size_t MAX_ITEM_SIZE = 32>
class StaticStack {
    alignas(T) uint8_t buffer[N][MAX_ITEM_SIZE] = {};
    uint16_t count = 0;

public:
    StaticStack() = default;

    StaticStack(const StaticStack&) = delete;
    StaticStack& operator=(const StaticStack&) = delete;

    ~StaticStack() {
        clear();
    }

    template <typename Derived>
    bool push(const Derived& value) {
        static_assert(sizeof(Derived) <= MAX_ITEM_SIZE, "Object size exceeds MAX_ITEM_SIZE");

        if (isFull()) return false;

        new (buffer[count]) Derived(value);

        count++;
        return true;
    }

    bool pop() {
        if (isEmpty()) return false;

        top().~T();

        --count;
        return true;
    }

    void clear() {
        while (pop());
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
