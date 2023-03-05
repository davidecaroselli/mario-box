//
// Created by Davide Caroselli on 05/03/23.
//

#include "Frame.h"
#include "graphics/olc/OLCFrame.h"

Frame *Frame::create(uint16_t width, uint16_t height) {
    return new OLCFrame(width, height);
}
