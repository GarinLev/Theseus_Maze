#include "Dist.h"

#include "Log.h"

#include <AceSorting.h>

void Dist::write_address() {
    unreset();
    delay(20);
    vl.setBus(wire);
    vl.setTimeout(1000);
    vl.setAddress(address);
    delay(10);

    if (!vl.init()) {
        LOG_ERROR("Initialization failed. Addr: ", address, ", pin: ", pin);
        initialized = false;
        return;
    }

    vl.startContinuous(25);
    initialized = true;
}

void Dist::init() const {
    pinMode(pin, OUTPUT);
    reset();
}

constexpr uint8_t STATUS_OK = 0x07;
constexpr uint16_t MAX_VALID_RANGE = 8000;

float Dist::get() {
    if (!initialized) return 0;
    uint8_t status = vl.readReg(VL53L0X::RESULT_INTERRUPT_STATUS);
    if (status & STATUS_OK) {
        uint16_t range = vl.readReg16Bit(VL53L0X::RESULT_RANGE_STATUS + 10);
        vl.writeReg(VL53L0X::SYSTEM_INTERRUPT_CLEAR, 0x01);

        if (range < MAX_VALID_RANGE) {

            arr_buff[arr_idx] = range;
            if (++arr_idx >= VL_DIST_ARRAY_LEN) {
                arr_idx = 0;
            }

            uint16_t dist_arr_sort[VL_DIST_ARRAY_LEN];
            memcpy(dist_arr_sort, arr_buff, sizeof(dist_arr_sort));
            ace_sorting::insertionSort(dist_arr_sort, VL_DIST_ARRAY_LEN);

            return dist_arr_sort[VL_DIST_ARRAY_LEN / 2];
        }
    }

    return 0;
}
