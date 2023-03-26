//
// Created by Davide Caroselli on 22/03/23.
//

#include "A2A03.h"
#include "synthesizer.h"

static double DUTYCYCLES[4] = {0.125, 0.250, 0.500, 0.725};
static uint8_t LENGTH_COUNTERS[32] = {
        10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

static uint16_t NOISE_PERIODS[16] = {
        4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708,  944, 1890, 3778
};

double A2A03::sample(double t) {
    double mix = 0;
    double norm = 0;

    if (pulse1.is_enabled()) {
        mix += pulse1.sample(t) * 0.7;
        norm += 1.;
    }
    if (pulse2.is_enabled()) {
        mix += pulse2.sample(t) * 0.7;
        norm += 1.;
    }
    if (triangle.is_enabled()) {
        mix += triangle.sample(t) * 0.7;
        norm += 1.;
    }
    if (noise.is_enabled()) {
        mix += noise.sample(t) * 0.5;
        norm += 1.;
    }

    return norm == 0. ? 0. : (mix / norm);
}

uint8_t A2A03::bus_read(uint8_t bus_id, uint16_t addr) {
    return 0;
}

void A2A03::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    pulse_t *pulse;

    switch (addr) {
        case 0x4000:
        case 0x4004:
            pulse = addr < 0x4004 ? &pulse1 : &pulse2;
            pulse->dutycycle = DUTYCYCLES[(val & 0b11000000) >> 6];
            pulse->envelope.loop = (val & 0b00100000) > 0;
            pulse->envelope.enabled = (val & 0b00010000) == 0;
            pulse->envelope.volume = (val & 0b00001111);
            pulse->length_counter.enabled = !pulse->envelope.enabled;
            break;
        case 0x4001:
        case 0x4005:
            pulse = addr < 0x4004 ? &pulse1 : &pulse2;
            pulse->sweep.enabled = (val & 0b10000000) > 0;
            pulse->sweep.period = (val & 0b01110000) >> 4;
            pulse->sweep.down = (val & 0b00001000) > 0;
            pulse->sweep.shift = val & 0b00000111;
            pulse->sweep.reload = true;
            break;
        case 0x4002:
        case 0x4006:
            pulse = addr < 0x4004 ? &pulse1 : &pulse2;
            pulse->timer = (pulse->timer & 0xFF00) | val;
            break;
        case 0x4003:
        case 0x4007:
            pulse = addr < 0x4004 ? &pulse1 : &pulse2;
            pulse->timer = ((uint16_t) (val & 0x07)) << 8 | (pulse->timer & 0x00FF);
            pulse->length_counter.counter = LENGTH_COUNTERS[val >> 3];
            pulse->envelope.start = true;
            break;

        case 0x4008:
            triangle.linear_counter.enabled = triangle.length_counter.enabled = (val & 0b10000000) == 0;
            triangle.linear_counter.reload_value = val & 0b01111111;
            break;
        case 0x4009:
            // unused
            break;
        case 0x400A:
            triangle.timer = (triangle.timer & 0xFF00) | val;
            break;
        case 0x400B:
            triangle.timer = ((uint16_t) (val & 0x07)) << 8 | (triangle.timer & 0x00FF);
            triangle.length_counter.counter = LENGTH_COUNTERS[val >> 3];
            triangle.linear_counter.reload = true;
            break;

        case 0x400C:
            noise.envelope.loop = (val & 0b00100000) > 0;
            noise.envelope.enabled = (val & 0b00010000) == 0;
            noise.envelope.volume = (val & 0b00001111);
            noise.length_counter.enabled = !noise.envelope.enabled;
            break;
        case 0x400D:
            // unused
            break;
        case 0x400E:
            noise.loop = (val & 0b10000000) > 0;
            noise.period = NOISE_PERIODS[(val & 0b00001111)];
            break;
        case 0x400F:
            noise.length_counter.counter = LENGTH_COUNTERS[val >> 3];
            noise.envelope.start = true;
            break;

        case 0x4015:
            pulse1.enabled = (val & 0x01) > 0;
            pulse2.enabled = (val & 0x02) > 0;
            triangle.enabled = (val & 0x04) > 0;
            noise.enabled = (val & 0x08) > 0;
            break;

    }
}

void A2A03::clock() {
    bool quarter_frame = false;
    bool half_frame = false;

    frame_ticks++;

    if (frame_ticks > 0 && frame_ticks % 3729 == 0)
        quarter_frame = true;
    if (frame_ticks > 0 && frame_ticks % 7458 == 0)
        half_frame = true;

    if (frame_ticks == 14916) frame_ticks = 0;

    pulse1.clock(quarter_frame, half_frame);
    pulse2.clock(quarter_frame, half_frame);
    triangle.clock(quarter_frame, half_frame);
    noise.clock(quarter_frame, half_frame);
}

// - Channel components ------------------------------------------------------------------------------------------------

void A2A03::envelope_t::clock() {
    if (start) {
        start = false;
        decay_count = 15;
        divider_count = volume;
    } else {
        if (divider_count == 0) {
            divider_count = volume;

            if (decay_count == 0) {
                if (loop)
                    decay_count = 15;
            } else {
                decay_count--;
            }
        } else {
            divider_count--;
        }
    }

    if (enabled)
        value = decay_count;
    else
        value = volume;
}

void A2A03::length_counter_t::clock() {
    if (counter > 0 && enabled)
        counter--;
}

void A2A03::linear_counter_t::clock() {
    if (reload)
        counter = reload_value;
    else if (counter > 0)
        counter--;

    if (enabled) reload = false;
}

bool A2A03::sweep_t::mute(uint16_t target) const {
    return enabled && (target < 8 || target > 0x7FF);
}

uint16_t A2A03::sweep_t::clock(uint16_t target) {
    if (!enabled) return target;

    uint16_t change = target >> shift;

    if (timer == 0 && shift > 0 && !mute(target)) {
        if (target >= 8 && change < 0x07FF) {
            if (down) {
                target -= change + channel;
            } else {
                target += change;
            }
        }
    }

    if (timer == 0 || reload) {
        timer = period;
        reload = false;
    } else {
        timer--;
    }

    return target;
}

// - Channels ----------------------------------------------------------------------------------------------------------

// - Pulse -

bool A2A03::pulse_t::is_enabled() const {
    return enabled && timer >= 8;
}

void A2A03::pulse_t::clock(bool quarter_frame, bool half_frame) {
    if (quarter_frame) envelope.clock();
    if (half_frame) length_counter.clock();
    if (half_frame) timer = sweep.clock(timer);
}

double A2A03::pulse_t::sample(double t) {
    if (length_counter.counter > 0 && envelope.value > 0 && !sweep.mute(timer)) {
        double f = 1662607. / (16. * (double) (timer + 1));
        return square_wave(t, f, dutycycle) * ((double) (envelope.value - 1) / 16.);
    } else {
        return 0.;
    }
}

// - Triangle -

bool A2A03::triangle_t::is_enabled() const {
    return enabled && timer >= 8;
}

void A2A03::triangle_t::clock(bool quarter_frame, bool half_frame) {
    if (half_frame) length_counter.clock();
    if (quarter_frame) linear_counter.clock();
}

double A2A03::triangle_t::sample(double t) {
    if (length_counter.counter > 0 && linear_counter.counter > 0) {
        double f = 1662607. / (32. * (double) (timer + 1));
        return triangle_wave(t, f);
    } else {
        return 0;
    }
}

// - Noise -

bool A2A03::noise_t::is_enabled() const {
    return enabled;
}

void A2A03::noise_t::clock(bool quarter_frame, bool half_frame) {
    if (quarter_frame) envelope.clock();
    if (half_frame) length_counter.clock();
}

double A2A03::noise_t::sample(double t) {
    if (length_counter.counter > 0 && envelope.value > 0) {
        const uint16_t step = 24;  // adjust sample speed

        for (int i = 0; i < step; ++i) {
            timer--;
            if (timer == 0xFFFF) {
                timer = period;

                uint8_t bit0 = sequence & 0x01;
                uint8_t bit1 = (sequence >> (loop ? 6 : 1)) & 0x01;
                sequence >>= 1;
                sequence |= (bit0 ^ bit1) << 14;
            }
        }

        double sample = 2. * ((double) (sequence & 0x01)) - 1.;
        return sample * ((double) (envelope.value - 1) / 16.);
    } else {
        return 0.;
    }
}
