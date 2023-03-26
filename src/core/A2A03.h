//
// Created by Davide Caroselli on 22/03/23.
//

#ifndef MARIOBOX_A2A03_H
#define MARIOBOX_A2A03_H

#include "SystemBus.h"

class A2A03 : public SBDevice {
public:
    [[nodiscard]] double sample(double t);

    void clock();

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

private:
    uint32_t frame_ticks = 0;

    // - Channel components -----------------------------------------------

    struct envelope_t {
        bool start = false;
        bool loop = false;
        bool enabled = false;

        uint8_t divider_count = 0;
        uint8_t volume = 0;
        uint8_t decay_count = 0;

        uint8_t value = 0;

        void clock();
    };

    struct length_counter_t {
        bool enabled = false;
        uint8_t counter = 0;

        void clock();
    };

    struct linear_counter_t {
        bool enabled = true;
        bool reload = false;
        uint8_t reload_value = 0;
        uint8_t counter = 0;

        void clock();
    };

    struct sweep_t {
        const uint8_t channel;

        bool enabled = false;
        bool down = false;
        bool reload = false;
        uint8_t shift = 0;
        uint8_t timer = 0;
        uint8_t period = 0;

        explicit sweep_t(uint8_t channel) : channel(channel & 0x01) {}

        uint16_t clock(uint16_t target);

        bool mute(uint16_t target) const;
    };

    // - Channels ---------------------------------------------------------

    struct channel_t {
        virtual void clock(bool quarter_frame, bool half_frame) = 0;

        [[nodiscard]] virtual bool is_enabled() const = 0;

        [[nodiscard]] virtual double sample(double t) = 0;
    };

    struct pulse_t : public channel_t {
        bool enabled = false;
        double dutycycle = 0;

        envelope_t envelope;
        length_counter_t length_counter;
        sweep_t sweep;

        uint16_t timer = 0;

        explicit pulse_t(uint8_t channel) : sweep(channel) {};

        void clock(bool quarter_frame, bool half_frame) override;

        [[nodiscard]] bool is_enabled() const override;

        [[nodiscard]] double sample(double t) override;

    };

    struct triangle_t : public channel_t {
        bool enabled = false;

        length_counter_t length_counter;
        linear_counter_t linear_counter;

        uint16_t timer = 0;

        void clock(bool quarter_frame, bool half_frame) override;

        [[nodiscard]] bool is_enabled() const override;

        [[nodiscard]] double sample(double t) override;
    };

    struct noise_t : public channel_t {
        bool enabled = false;
        bool loop = false;
        uint16_t period = 0;

        envelope_t envelope;
        length_counter_t length_counter;

        uint16_t timer = 0;
        uint16_t sequence = 1;

        void clock(bool quarter_frame, bool half_frame) override;

        [[nodiscard]] bool is_enabled() const override;

        [[nodiscard]] double sample(double t) override;
    };

    pulse_t pulse1 = pulse_t(0);
    pulse_t pulse2 = pulse_t(1);
    triangle_t triangle;
    noise_t noise;

};


#endif //MARIOBOX_A2A03_H
