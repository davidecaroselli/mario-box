//
// Created by Davide Caroselli on 22/03/23.
//

#include "synthesizer.h"


// - Utils -------------------------------------------------------------------------------------------------------------

#define PI 3.141592653589793
#define HALF_PI 1.570796326794897
#define DOUBLE_PI 6.283185307179586
#define SIN_CURVE_A 0.0415896
#define SIN_CURVE_B 0.00129810625032

// Thanks to Josh at https://stackoverflow.com/a/55577974/2676776
double approx_cos(double x) {
    if (x < 0) {
        int q = -(int) (x / DOUBLE_PI);
        q += 1;
        double y = q * DOUBLE_PI;
        x = -(x - y);
    }
    if (x >= DOUBLE_PI) {
        int q = (int) (x / DOUBLE_PI);
        double y = q * DOUBLE_PI;
        x = x - y;
    }
    int s = 1;
    if (x >= PI) {
        s = -1;
        x -= PI;
    }
    if (x > HALF_PI) {
        x = PI - x;
        s = -s;
    }
    double z = x * x;
    double r = z * (z * (SIN_CURVE_A - SIN_CURVE_B * z) - 0.5) + 1.0;
    if (r > 1.0) r = r - 2.0;
    if (s > 0) return r;
    else return -r;
}

double approx_sin(double x) {
    return approx_cos(x - HALF_PI);
}

// ---------------------------------------------------------------------------------------------------------------------

double square_wave(double t, double tone, double dutycycle, double amplitude, int harmonics) {
    double a = 0;
    double b = 0;
    double p = dutycycle * DOUBLE_PI;

    for (int n_ = 0; n_ < harmonics; ++n_) {
        double n = (double) n_ + 1;
        double c = n * t * tone;
        a += -approx_sin(c) / n;
        b += -approx_sin(c - p * n) / n;
    }

    return (2.0 * amplitude / PI) * (a - b);
}