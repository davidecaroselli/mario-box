//
// Created by Davide Caroselli on 08/03/23.
//

#ifndef MARIOBOX_NAMETABLES_H
#define MARIOBOX_NAMETABLES_H


#include "core/Memory.h"

enum Mirroring {
    Vertical,
    Horizontal
};

class NameTables: public SBDevice {
public:
    Mirroring mirroring = Vertical;

    uint16_t bus_begin(uint8_t bus_id) override {
        return 0x2000;
    }

    uint16_t bus_end(uint8_t bus_id) override {
        return 0x2FFF;
    }

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

    // - Debug utilities ---------------------------------------------------

    uint8_t at(uint8_t table, uint8_t x, uint8_t y) {
        return data[table & 0x01][y * 32 + x];
    }

private:
    uint8_t data[2][1024] = {};

    uint16_t map(uint16_t addr, uint8_t *out_table) const;
};


#endif //MARIOBOX_NAMETABLES_H
