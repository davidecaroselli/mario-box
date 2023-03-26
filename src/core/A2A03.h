//
// Created by Davide Caroselli on 22/03/23.
//

#ifndef MARIOBOX_A2A03_H
#define MARIOBOX_A2A03_H

#include "SystemBus.h"

class A2A03 : public SBDevice {
public:
    [[nodiscard]] double sample(double t) const;

    void clock();

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

private:
    uint32_t frame_ticks = 0;

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

    struct length_t {
        bool enabled = false;
        uint8_t counter = 0;

        uint8_t value = 0;

        void clock();
    };

    struct sweeper_t
    {
        bool enabled = false;
        bool down = false;
        bool reload = false;
        uint8_t shift = 0x00;
        uint8_t timer = 0x00;
        uint8_t period = 0x00;
        uint16_t change = 0;
        bool mute = false;

        void track(uint16_t &target)
        {
            if (enabled)
            {
                change = target >> shift;
                mute = (target < 8) || (target > 0x7FF);
            }
        }

        bool clock(uint16_t &target, bool channel)
        {
            bool changed = false;
            if (timer == 0 && enabled && shift > 0 && !mute)
            {
                if (target >= 8 && change < 0x07FF)
                {
                    if (down)
                    {
                        target -= change - channel;
                    }
                    else
                    {
                        target += change;
                    }
                    changed = true;
                }
            }

            if (enabled)
            {
                if (timer == 0 || reload)
                {
                    timer = period;
                    reload = false;
                }
                else
                    timer--;

                mute = (target < 8) || (target > 0x7FF);
            }

            return changed;
        }
    };

    struct channel_t {
        virtual void clock(bool quarter_frame, bool half_frame) = 0;

        virtual bool is_enabled() const = 0;

        virtual double sample(double t) const = 0;
    };

    struct pulse_t : public channel_t {
        const uint8_t channel;

        bool enabled = false;
        double dutycycle = 0;

        envelope_t envelope;
        length_t length;
        sweeper_t sweeper;

        uint16_t timer = 0;

        explicit pulse_t(uint8_t channel) : channel(channel) {};

        void clock(bool quarter_frame, bool half_frame) override;

        bool is_enabled() const override;

        double sample(double t) const override;

    };

    struct triangle_t : public channel_t {
        bool enabled = false;

        length_t length;
        bool linear_counter_enabled = false;
        bool linear_counter_reload = false;
        uint8_t linear_counter_reload_val = 0;
        uint8_t linear_counter = 0;

        uint16_t timer = 0;

        void clock(bool quarter_frame, bool half_frame) override;

        bool is_enabled() const override;

        double sample(double t) const override;
    };

    pulse_t pulse1 = pulse_t(0);
    pulse_t pulse2 = pulse_t(1);
    triangle_t triangle;

};


#endif //MARIOBOX_A2A03_H
