//
// Created by Davide Caroselli on 22/03/23.
//

#include "A2A03.h"
#include "synthesizer.h"

double A2A03::sample(double t) {
    return (square_wave(t, demo_sw1_tone) + square_wave(t, demo_sw2_tone)) / 2.;
}
