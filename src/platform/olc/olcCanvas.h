//
// Created by Davide Caroselli on 05/03/23.
//

#ifndef MARIOBOX_OLCCANVAS_H
#define MARIOBOX_OLCCANVAS_H

#include "platform/Canvas.h"
#include "olcPixelGameEngine.h"

class olcCanvas: public Canvas {
public:
    olc::Sprite sprite;

    olcCanvas(uint16_t width, uint16_t height);

    void set(uint16_t x, uint16_t y, const NESColor *color) override;
};


#endif //MARIOBOX_OLCCANVAS_H
