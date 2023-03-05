//
// Created by Davide Caroselli on 05/03/23.
//

#include "Palettes.h"

uint16_t Palettes::bus_begin(uint8_t bus_id) {
    return 0x3F00;
}

uint16_t Palettes::bus_end(uint8_t bus_id) {
    return 0x3FFF;
}

uint8_t Palettes::bus_read(uint8_t bus_id, uint16_t addr) {
    addr &= 0x001F;
    if (addr == 0x3F10) addr = 0x3F00;
    else if (addr == 0x3F14) addr = 0x3F04;
    else if (addr == 0x3F18) addr = 0x3F08;
    else if (addr == 0x3F1C) addr = 0x3F0C;

    return data[addr];
}

void Palettes::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    addr &= 0x001F;
    if (addr == 0x3F10) addr = 0x3F00;
    else if (addr == 0x3F14) addr = 0x3F04;
    else if (addr == 0x3F18) addr = 0x3F08;
    else if (addr == 0x3F1C) addr = 0x3F0C;

    data[addr] = val;
}

const NESColor &Palettes::color_at(uint8_t palette, uint8_t offset) {
    uint8_t cid = bus_read(0, (palette << 2) + offset);
    return NESColors[cid & 0x37];
}
