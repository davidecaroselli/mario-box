//
// Created by Davide Caroselli on 08/03/23.
//

#include "NameTables.h"

uint16_t NameTables::map(uint16_t addr, uint8_t *out_table) const {
    addr &= 0x0FFF;

    if (mirroring == Vertical) {
        *out_table = (addr >> 10) & 0x01;
    } else if (mirroring == Horizontal) {
        if (0x000 <= addr && addr < 0x800)
            *out_table = 0;
        else
            *out_table = 1;
    }

    return addr & 0x3FF;
}

uint8_t NameTables::bus_read(uint8_t bus_id, uint16_t addr) {
    uint8_t table;
    uint16_t offset = map(addr, &table);
    return data[table][offset];
}

void NameTables::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    uint8_t table;
    uint16_t offset = map(addr, &table);
    data[table][offset] = val;
}
