//
// Created by Davide Caroselli on 05/03/23.
//

#ifndef MARIOBOX_PALETTES_H
#define MARIOBOX_PALETTES_H


#include "core/Memory.h"
#include "graphics/NESColor.h"

class Palettes : public SBDevice {
public:
    const NESColor &color_at(uint8_t palette, uint8_t offset);

    uint16_t bus_begin(uint8_t bus_id) override;

    uint16_t bus_end(uint8_t bus_id) override;

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

private:
    uint8_t data[32]{};
};


#endif //MARIOBOX_PALETTES_H
