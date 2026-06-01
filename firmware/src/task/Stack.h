#ifndef FIRMWARE_STACK_H
#define FIRMWARE_STACK_H

template <typename T, uint16_t N, size_t MAX_ITEM_SIZE = 16>
class StaticStack {
    alignas(void*) uint8_t buffer[N][MAX_ITEM_SIZE] = {};
    uint16_t count = 0;

public:
    StaticStack() = default;

    template <typename Derived>
    bool push(const Derived& value) {
        static_assert(sizeof(Derived) <= MAX_ITEM_SIZE, "Object size exceeds MAX_ITEM_SIZE");

        if (isFull()) return false;

        auto src = reinterpret_cast<const uint8_t*>(&value);
        uint8_t* dst = buffer[count];
        for (size_t i = 0; i < sizeof(Derived); ++i) {
            dst[i] = src[i];
        }

        count++;
        return true;
    }

    bool pop() {
        if (isEmpty()) return false;
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
