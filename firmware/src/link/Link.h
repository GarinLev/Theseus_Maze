#ifndef FIRMWARE_LINK_H
#define FIRMWARE_LINK_H

#include <HardwareSerial.h>

#include "DebugLogEnable.h"
#include "Log.h"

class Link
{
public:
    explicit Link(HardwareSerial *_serial_base, HardwareSerial *_serial_debug)
        : serial_base(_serial_base), serial_debug(_serial_debug) {}

    void wait_start() const;

    void update() const;
    void update_debug() const;

    void send_sensors(const bool distance[4], uint8_t color) const;
    void log_debug(const char* message) const;
    void pause(bool pause) const {
        if (pause) {
            serial_base->write('p');
            LOG_INFO('p');
        } else {
            serial_base->write('o');
            LOG_INFO('o');
        }
    }

private:
    HardwareSerial* serial_base;
    HardwareSerial* serial_debug;

    static void process_command(char cmd);
};

#endif