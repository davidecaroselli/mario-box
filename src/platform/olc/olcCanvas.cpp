//
// Created by Davide Caroselli on 05/03/23.
//

#include "olcCanvas.h"

olcCanvas::olcCanvas(uint16_t width, uint16_t height) : Canvas(width, height), sprite(width, height) {}

void olcCanvas::set(uint16_t x, uint16_t y, const NESColor &color) {
    olc::Pixel p(color.r, color.g, color.b);
    sprite.SetPixel(x, y, p);
}
