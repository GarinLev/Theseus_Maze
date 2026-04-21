#pragma once

#include "../../lib/pt/pt.h"
#include "../macro.h"
#include "../../lib/VL53L0X/VL53L0X_mod.h"
#include "../../lib/Optional/Optional.h"

#define DIST_ARRAY_LEN 5

struct DistNode {
    struct pt pt;

    VL53L0X_mod lox;
    uint8_t addr;
    uint8_t pin_sht;
    TwoWire* wire;

    uint32_t dt;
    uint32_t last_time;
    
    Optional<uint16_t> dist;

    uint8_t dist_arr_idx = 0;
    uint16_t dist_arr_buff[DIST_ARRAY_LEN] = {};
};

void node_dist_init(DistNode&ctx);
int  node_dist_run(DistNode&ctx);