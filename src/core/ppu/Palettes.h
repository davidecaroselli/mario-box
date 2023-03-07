//
// Created by Davide Caroselli on 05/03/23.
//

#ifndef MARIOBOX_PALETTES_H
#define MARIOBOX_PALETTES_H


#include "core/Memory.h"
#include "NESColor.h"
#include "platform/Canvas.h"

struct Palette {
    const NESColor *colors[4] = {nullptr, nullptr, nullptr, nullptr};
};

class Palettes : public SBDevice {
public:
    uint16_t bus_begin(uint8_t bus_id) override;

    uint16_t bus_end(uint8_t bus_id) override;

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

    // - Debug utilities ---------------------------------------------------

    void render(uint8_t palette, Canvas *canvas);

    Palette get(uint8_t palette);

private:
    uint8_t data[32]{};

    const NESColor *color_at(uint8_t palette, uint8_t offset);
};


#endif //MARIOBOX_PALETTES_H
