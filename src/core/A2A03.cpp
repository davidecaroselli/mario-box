//
// Created by Davide Caroselli on 22/03/23.
//

#include "A2A03.h"
#include "synthesizer.h"

double A2A03::sample(double t) const {
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
        mix += triangle.sample(t) * 0.5;
        norm += 1.;
    }

    return norm == 0. ? 0. : (mix / norm);
}

uint8_t A2A03::bus_read(uint8_t bus_id, uint16_t addr) {
    return 0;
}

static double DUTYCYCLES[4] = {0.125, 0.250, 0.500, 0.725};
static uint8_t LENGTH_COUNTERS[32] = {
        10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

void A2A03::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    pulse_t *pulse;

    switch (addr) {
        case 0x4000:
        case 0x4004:
            pulse = addr < 0x4004 ? &pulse1 : &pulse2;
            pulse->dutycycle = DUTYCYCLES[(val & 0b11000000) >> 6];
            pulse->length.enabled = (val & 0b00010000) > 0;
            pulse->envelope.loop = (val & 0b00100000) > 0;
            pulse->envelope.enabled = !pulse->length.enabled;
            pulse->envelope.volume = (val & 0b00001111);
            break;
        case 0x4001:
        case 0x4005:
            pulse = addr < 0x4004 ? &pulse1 : &pulse2;
            pulse->sweeper.enabled = val & 0x80;
            pulse->sweeper.period = (val & 0x70) >> 4;
            pulse->sweeper.down = val & 0x08;
            pulse->sweeper.shift = val & 0x07;
            pulse->sweeper.reload = true;
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
            pulse->length.counter = LENGTH_COUNTERS[val >> 3];
            pulse->envelope.start = true;
            break;

        case 0x4008:
            triangle.linear_counter_enabled = triangle.length.enabled = (val & 0b10000000) == 0;
            triangle.linear_counter_reload_val = val & 0b01111111;
            break;
        case 0x4009:
            break;
        case 0x400A:
            triangle.timer = (triangle.timer & 0xFF00) | val;
            break;
        case 0x400B:
            triangle.timer = ((uint16_t) (val & 0x07)) << 8 | (triangle.timer & 0x00FF);
            triangle.length.counter = LENGTH_COUNTERS[val >> 3];
            triangle.linear_counter_reload = true;
            break;

        case 0x4015:
            pulse1.enabled = (val & 0x01) > 0;
            pulse2.enabled = (val & 0x02) > 0;
            triangle.enabled = (val & 0x04) > 0;
            break;

    }
}

void A2A03::clock() {
    bool quarter_frame = false;
    bool half_frame = false;

    frame_ticks++;

    if (frame_ticks == 3729) {
        quarter_frame = true;
    } else if (frame_ticks == 7457) {
        quarter_frame = true;
        half_frame = true;
    } else if (frame_ticks == 11186) {
        quarter_frame = true;
    } else if (frame_ticks == 14916) {
        quarter_frame = true;
        half_frame = true;
        frame_ticks = 0;
    }

    pulse1.clock(quarter_frame, half_frame);
    pulse2.clock(quarter_frame, half_frame);
    triangle.clock(quarter_frame, half_frame);
}

// - Channels ----------------------------------------------------------------------------------------------------------

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

void A2A03::length_t::clock() {
    if (counter > 0 && enabled)
        counter--;
}

bool A2A03::pulse_t::is_enabled() const {
    return enabled && timer >= 8;
}

double A2A03::pulse_t::sample(double t) const {
    if (length.counter > 0 && envelope.value > 0 && !sweeper.mute) {
        double f = 1662607. / (16. * (double) (timer + 1));
        return square_wave(t, f, dutycycle) * ((double) envelope.value / 15.);
    } else {
        return 0.;
    }
}

void A2A03::pulse_t::clock(bool quarter_frame, bool half_frame) {
    if (quarter_frame) envelope.clock();
    if (half_frame) length.clock();
    if (half_frame) sweeper.clock(timer, channel);

    sweeper.track(timer);
}

void A2A03::triangle_t::clock(bool quarter_frame, bool half_frame) {
    if (half_frame) length.clock();
    if (quarter_frame) {
        if (linear_counter_reload) {
            linear_counter = linear_counter_reload_val;
        } else if (linear_counter > 0) {
            linear_counter--;
        }

        if (linear_counter_enabled) linear_counter_reload = false;
    }
}

bool A2A03::triangle_t::is_enabled() const {
    return enabled && timer >= 2;
}

double A2A03::triangle_t::sample(double t) const {
    if (length.counter > 0 && linear_counter > 0) {
        double f = 1662607. / (32. * (double) (timer + 1));
        return triangle_wave(t, f);
    } else {
        return 0;
    }
}
