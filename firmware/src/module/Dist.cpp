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

    delay(30);
    uint16_t start_range = vl.readRangeContinuousMillimeters();
    if (start_range > 8000 || start_range == 0) {
        start_range = 150;
    }

    for (uint16_t & i : arr_buff) {
        i = start_range;
    }
    last_median = start_range;
    last_update_time = millis();

    delay(20);
}
void Dist::init() const {
    pinMode(pin, OUTPUT);
    reset();
}

constexpr uint16_t MAX_VALID_RANGE = 8000;

void Dist::update() {
    if (!initialized) return;

    if ((vl.readReg(VL53L0X::RESULT_INTERRUPT_STATUS) & 0x07) != 0) {

        uint16_t range = vl.readReg16Bit(VL53L0X::RESULT_RANGE_STATUS + 10);
        vl.writeReg(VL53L0X::SYSTEM_INTERRUPT_CLEAR, 0x01);

        if (range < MAX_VALID_RANGE) {
            arr_buff[arr_idx] = range;
            if (++arr_idx >= VL_DIST_ARRAY_LEN) {
                arr_idx = 0;
            }

            uint16_t dist_arr_sort[VL_DIST_ARRAY_LEN];

            for (size_t i = 0; i < VL_DIST_ARRAY_LEN; ++i) {
                if (arr_buff[i] == 0) {
                    dist_arr_sort[i] = MAX_VALID_RANGE;
                } else {
                    dist_arr_sort[i] = arr_buff[i];
                }
            }

            ace_sorting::insertionSort(dist_arr_sort, VL_DIST_ARRAY_LEN);

            size_t mid_idx = VL_DIST_ARRAY_LEN / 2;
            uint16_t median_candidate = dist_arr_sort[mid_idx];

            if (median_candidate == MAX_VALID_RANGE) {
                last_median = 0;
            } else {
                last_median = median_candidate;
            }

            last_update_time = millis();
        }
    }
}

float Dist::get() const {
    if (!initialized) return 0;

    if (millis() - last_update_time > 150) {
        return 0;
    }

    return last_median;
}
