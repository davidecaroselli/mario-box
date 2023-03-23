//
// Created by Davide Caroselli on 22/03/23.
//

#include "A2A03.h"
#include "synthesizer.h"

double A2A03::sample(double t) {
//    return (square_wave(t, demo_sw1_tone) + square_wave(t, demo_sw2_tone)) / 2.;
}

uint8_t A2A03::bus_read(uint8_t bus_id, uint16_t addr) {
    return 0;
}

void A2A03::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {

}
