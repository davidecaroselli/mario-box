#include <iostream>
#include "hardware/bus/Bus.h"
#include "hardware/C6502.h"

int main() {
    std::shared_ptr<Bus> bus(new Bus());

    bus->write(PROG_BEGIN_PTR, 0x00);
    bus->write(PROG_BEGIN_PTR + 1, 0x80);

    uint16_t addr = 0x8000;
    bus->write(addr++, 0xA2);
    bus->write(addr++, 0x0A);
    bus->write(addr++, 0x8E);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0xA2);
    bus->write(addr++, 0x03);
    bus->write(addr++, 0x8E);
    bus->write(addr++, 0x01);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0xAC);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0xA9);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0x18);
    bus->write(addr++, 0x6D);
    bus->write(addr++, 0x01);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0x88);
    bus->write(addr++, 0xD0);
    bus->write(addr++, 0xFA);
    bus->write(addr++, 0x8D);
    bus->write(addr++, 0x02);
    bus->write(addr++, 0x00);
    bus->write(addr++, 0xEA);
    bus->write(addr++, 0xEA);
    bus->write(addr++, 0xEA);

    C6502 cpu(bus);
    cpu.reset();
    cpu.printf();

    while(true) {
        cpu.step();
    }

    return 0;
}
