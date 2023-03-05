//
// Created by Davide Caroselli on 05/03/23.
//

#include "OLCFrame.h"
//#include "olcPixelGameEngine.h"

OLCFrame::OLCFrame(uint16_t width, uint16_t height) : Frame(width, height) {
}

void OLCFrame::set(uint16_t x, uint16_t y, const NESColor &color) {
    //((olc::Sprite *) sprite)->SetPixel(x, y, olc::Pixel(color.r, color.g, color.b, color.a));
}
