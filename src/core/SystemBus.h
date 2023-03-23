//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_SYSTEMBUS_H
#define MARIOBOX_SYSTEMBUS_H

#include <cstdint>
#include <memory>
#include <vector>

class SystemBus;

class SBDevice {
public:
    virtual uint8_t bus_read(uint8_t bus_id, uint16_t addr) = 0;

    virtual void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) = 0;
};

class SystemBus {
public:
    static const uint8_t MAIN_BUS_ID = 0x01;
    static const uint8_t PPU_BUS_ID = 0x02;

    const uint8_t id;

    explicit SystemBus(uint8_t id) : id(id) {}

    void connect(uint16_t begin, uint16_t end, SBDevice *device);

    void disconnect(SBDevice *device);

    uint8_t read(uint16_t addr);

    void write(uint16_t addr, uint8_t val);

private:
    struct sb_device_t {
        uint16_t begin;
        uint16_t end;
        SBDevice *device;

        sb_device_t(uint16_t begin, uint16_t end, SBDevice *device)
                : begin(begin), end(end), device(device) {};
    };

    std::vector<sb_device_t> devices;
};

#endif //MARIOBOX_SYSTEMBUS_H
