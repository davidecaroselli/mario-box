//
// Created by Davide Caroselli on 08/03/23.
//

#include "Controllers.h"

uint8_t Controllers::bus_read(uint8_t bus_id, uint16_t addr) {
    uint8_t val = (buffer[addr & 0x0001] & 0x80) > 0 ? 0x01 : 0x00;
    buffer[addr & 0x0001] <<= 1;
    return val;
}

void Controllers::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    if (addr == 0x4016 && (val & 0x01) == 0) {
        buffer[0] = buttons[0];
        buffer[1] = buttons[1];
    }
}
