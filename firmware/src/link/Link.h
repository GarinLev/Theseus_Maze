#ifndef FIRMWARE_LINK_H
#define FIRMWARE_LINK_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "DebugLogEnable.h"
#include "Log.h"

class Link {
public:
    Link(HardwareSerial* base, HardwareSerial* debug)
        : serial_base(base), serial_debug(debug) {}

    void wait_start() const;
    void update() const;
    void update_debug() const;

    void send_sensors(const bool distance[4], uint8_t color) const;
    void log_debug(const char* message) const;
    void send_pause(bool paused) const;

private:
    HardwareSerial* serial_base;
    HardwareSerial* serial_debug;

    void read_from_stream(Stream* stream, const char* label) const;
    static void process_command(char cmd);
};

#endif // FIRMWARE_LINK_H