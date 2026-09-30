#ifndef FIRMWARE_LINK_H
#define FIRMWARE_LINK_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "DebugLogEnable.h"
#include "Log.h"
class Robot;

class Link {
public:
    Link(HardwareSerial* base, HardwareSerial* debug)
        : serial_base(base), serial_debug(debug) {}

    void wait_start() const;
    void update();
    void update_debug();
    void update_command_queue();

    void send_sensors(const bool distance[4], uint8_t color) const;
    void log_debug(const char* message) const;
    void send_pause(bool paused) const;
    void push_forward_sequence(Robot& robot);


    String serial_buffer;
private:
    HardwareSerial* serial_base;
    HardwareSerial* serial_debug;
    char command_queue[16];
    uint8_t queue_head = 0;
    uint8_t queue_tail = 0;

    void read_from_stream(Stream* stream, const char* label);
    void process_command(char cmd);
    void execute_command(char cmd);
};

#endif // FIRMWARE_LINK_H