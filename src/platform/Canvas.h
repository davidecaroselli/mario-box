//
// Created by Davide Caroselli on 05/03/23.
//

#ifndef MARIOBOX_CANVAS_H
#define MARIOBOX_CANVAS_H

#include <cstdint>
#include "core/ppu/NESColor.h"

class Canvas {
public:
    const uint16_t width;
    const uint16_t height;

    Canvas(const uint16_t width, const uint16_t height) : width(width), height(height) {}

    virtual ~Canvas() = default;

    virtual void set(uint16_t x, uint16_t y, const NESColor *color) = 0;
};

#endif //MARIOBOX_CANVAS_H
