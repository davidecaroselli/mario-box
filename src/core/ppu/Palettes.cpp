//
// Created by Davide Caroselli on 05/03/23.
//

#include "Palettes.h"

const std::vector<NESColor> PAL = {
         {84 , 84 , 84 }, {0  , 30 , 116}, {8  , 16 , 144}, {48 , 0  , 136}, {68 , 0  , 100}, {92 , 0  , 48 }, {84 , 4  , 0  }, {60 , 24 , 0  }, {32 , 42 , 0  }, {8  , 58 , 0  }, {0  , 64 , 0  }, {0  , 60 , 0  }, {0  , 50 , 60 }, {0  , 0  , 0  }, {0  , 0  , 0  }, {0  , 0  , 0  },
         {152, 150, 152}, {8  , 76 , 196}, {48 , 50 , 236}, {92 , 30 , 228}, {136, 20 , 176}, {160, 20 , 100}, {152, 34 , 32 }, {120, 60 , 0  }, {84 , 90 , 0  }, {40 , 114, 0  }, {8  , 124, 0  }, {0  , 118, 40 }, {0  , 102, 120}, {0  , 0  , 0  }, {0  , 0  , 0  }, {0  , 0  , 0  },
         {236, 238, 236}, {76 , 154, 236}, {120, 124, 236}, {176, 98 , 236}, {228, 84 , 236}, {236, 88 , 180}, {236, 106, 100}, {212, 136, 32 }, {160, 170, 0  }, {116, 196, 0  }, {76 , 208, 32 }, {56 , 204, 108}, {56 , 180, 204}, {60 , 60 , 60 }, {0  , 0  , 0  }, {0  , 0  , 0  },
         {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {0  , 0  , 0  }, {0  , 0  , 0  }
};

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
    if (addr == 0x3F10) addr = 0x3F00;
    else if (addr == 0x3F14) addr = 0x3F04;
    else if (addr == 0x3F18) addr = 0x3F08;
    else if (addr == 0x3F1C) addr = 0x3F0C;
    addr &= 0x001F;
    data[addr] = val;
}

const NESColor *Palettes::color_at(uint8_t palette, uint8_t offset) {
    uint8_t cid = bus_read(0, (palette << 2) + offset);
    assert(cid < 64);
    return &PAL[cid];
}

void Palettes::render(uint8_t palette, Canvas *canvas) {
    assert(canvas->width == 4);
    assert(canvas->height == 1);
    canvas->set(0, 0, color_at(palette, 0));
    canvas->set(1, 0, color_at(palette, 1));
    canvas->set(2, 0, color_at(palette, 2));
    canvas->set(3, 0, color_at(palette, 3));
}

Palette Palettes::get(uint8_t palette) {
    Palette ret;
    ret.colors[0] = color_at(palette, 0);
    ret.colors[1] = color_at(palette, 1);
    ret.colors[2] = color_at(palette, 2);
    ret.colors[3] = color_at(palette, 3);
    return ret;
}
