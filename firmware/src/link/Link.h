#ifndef FIRMWARE_LINK_H
#define FIRMWARE_LINK_H

#include <HardwareSerial.h>

class Link
{
public:
    explicit Link(HardwareSerial *_serial) : serial(_serial) {}

    void init() const;
    void wait_start() const;
    void update() const;

    void send_sensors(bool distance[4], uint8_t color) const;

private:
    HardwareSerial* serial;
};

#endif //FIRMWARE_LINK_H