//
// Created by Davide Caroselli on 07/03/23.
//

#ifndef MARIOBOX_PATTERNTABLES_H
#define MARIOBOX_PATTERNTABLES_H

#include "core/Memory.h"
#include "platform/Canvas.h"
#include "Palettes.h"

class PatternTables: public Memory {
public:
    PatternTables(const std::vector<uint8_t> &memory, const std::shared_ptr<Mapper> &mapper);

    // - Debug utilities ---------------------------------------------------

    void render(uint8_t table, const Palette &palette, Canvas *canvas);

    void render_tile(uint8_t table, uint8_t tile, const Palette &palette, Canvas *canvas, uint8_t x, uint8_t y);
};


#endif //MARIOBOX_PATTERNTABLES_H
