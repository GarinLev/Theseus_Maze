#pragma once
#include <Arduino.h>

const uint8_t PROTOCOL_START_BYTE = 0xAB;
const uint8_t MAX_PACKET_SIZE = 20;

const uint8_t PROTOCOL_RESP_START_BITS = 0x0A;
const uint8_t PROTOCOL_SHIFT = 4;
const uint8_t PROTOCOL_MASK = 0x0F;

enum Command : uint8_t {
    CMD_PING = 0b0001,
    CMD_MOVE = 0b0010,
    CMD_SEND_SENSORS = 0b0011,
    CMD_SET_KIT = 0b0100,
    CMD_COLOR_NOTIFY = 0b0101,
    CMD_STATUS_REQ = 0b0110,
    CMD_TASK_DONE = 0b0111
};

enum Response : uint8_t {
    RESP_PONG = 0b1111,
    RESP_OK = 0b1001,
    RESP_SENSORS = 0b1010,
    RESP_STATE = 0b1011,
    RESP_ERR_STRUCT = 0b1110,
    RESP_ERR_CRC = 0b1101
};

struct __attribute__((__packed__)) SensorsPayload {
    uint16_t dist[6];
    uint8_t color;
};