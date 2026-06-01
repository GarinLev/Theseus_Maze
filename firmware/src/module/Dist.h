#ifndef FIRMWARE_DIST_H
#define FIRMWARE_DIST_H

#include <Wire.h>
#include <VL53L0X.h>

constexpr uint8_t VL_DIST_ARRAY_LEN = 3;
class Dist {
public:
    explicit Dist(uint8_t pin, uint8_t _address)
    : wire(&Wire), pin(pin), address(_address) {}

    void init() const;
    void write_address();
    void reset() const { digitalWrite(pin, LOW); delay(20); }
    void unreset() const { digitalWrite(pin, HIGH); delay(20); };
    float get();

private:
    VL53L0X vl;
    TwoWire *wire = nullptr;
    uint8_t pin, address;

    bool initialized = false;
    uint8_t arr_idx = 0;
    uint16_t arr_buff[VL_DIST_ARRAY_LEN] = {};
    float last_median = 0;
};


#endif //FIRMWARE_DIST_H