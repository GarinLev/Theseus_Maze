#ifndef FIRMWARE_LINK_H
#define FIRMWARE_LINK_H

#include <HardwareSerial.h>

class Link
{
public:
    // Конструктор принимает основной порт управления и порт отладки
    explicit Link(HardwareSerial *_serial_base, HardwareSerial *_serial_debug)
        : serial_base(_serial_base), serial_debug(_serial_debug) {}

    void wait_start() const;

    void update() const;
    void update_debug() const;

    void send_sensors(const bool distance[4], uint8_t color) const;
    void log_debug(const char* message) const;
    void pause(bool pause) const { serial_base->print(pause ? 'p' : 'o'); }

private:
    HardwareSerial* serial_base;
    HardwareSerial* serial_debug;

    static void process_command(char cmd);
};

#endif //FIRMWARE_LINK_H