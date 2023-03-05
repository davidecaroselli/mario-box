//
// Created by Davide Caroselli on 05/03/23.
//

#ifndef MARIOBOX_OLCFRAME_H
#define MARIOBOX_OLCFRAME_H

#include "graphics/Frame.h"

class OLCFrame: public Frame {
public:
    OLCFrame(uint16_t width, uint16_t height);

    void set(uint16_t x, uint16_t y, const NESColor &color) override;
};


#endif //MARIOBOX_OLCFRAME_H
