//
// Created by Davide Caroselli on 22/03/23.
//

#ifndef MARIOBOX_SYNTHESIZER_H
#define MARIOBOX_SYNTHESIZER_H

double square_wave(double t, double freq, double dutycycle = .5, unsigned int harmonics = 20);

double triangle_wave(double t, double freq, unsigned int harmonics = 10);

#endif //MARIOBOX_SYNTHESIZER_H
