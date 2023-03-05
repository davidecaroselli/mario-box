//
// Created by Davide Caroselli on 05/03/23.
//

#ifndef MARIOBOX_FRAME_H
#define MARIOBOX_FRAME_H

#include <cstdint>
#include <vector>
#include "NESColor.h"

class Frame {
public:
    static Frame *create(uint16_t width, uint16_t height);

    const uint16_t width;
    const uint16_t height;

    Frame(uint16_t width, uint16_t height): width(width), height(height) {};

    virtual ~Frame() = default;

    virtual void set(uint16_t x, uint16_t y, const NESColor &color) = 0;
};

#endif //MARIOBOX_FRAME_H
