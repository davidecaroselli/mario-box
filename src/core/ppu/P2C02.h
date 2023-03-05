//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_P2C02_H
#define MARIOBOX_P2C02_H

#include "core/SystemBus.h"
#include "Palettes.h"
#include "core/C6502.h"
#include "platform/Canvas.h"

union ppustatus_t {
    struct {
        uint8_t _: 5;
        uint8_t sprite_overflow: 1;
        uint8_t sprite_zero_hit: 1;
        uint8_t vertical_blank: 1;
    };
    uint8_t reg = 0;
};

union ppumask_t {
    struct {
        uint8_t grayscale: 1;
        uint8_t render_background_left: 1;
        uint8_t render_sprites_left: 1;
        uint8_t render_background: 1;
        uint8_t render_sprites: 1;
        uint8_t enhance_red: 1;
        uint8_t enhance_green: 1;
        uint8_t enhance_blue: 1;
    };
    uint8_t reg = 0;
};

union ppucontrol_t {
    struct {
        uint8_t nametable_x: 1;
        uint8_t nametable_y: 1;
        uint8_t increment_mode: 1;
        uint8_t pattern_sprite: 1;
        uint8_t pattern_background: 1;
        uint8_t sprite_size: 1;
        uint8_t slave_mode: 1;
        uint8_t enable_nmi: 1;
    };
    uint8_t reg = 0;
};

class P2C02: public SBDevice {
public:
    SystemBus bus;
    Palettes palettes;
    Memory name_tables;

    ppustatus_t status;
    ppumask_t mask;
    ppucontrol_t control;

    explicit P2C02(Canvas *screen);

    [[nodiscard]] bool nmi() const {
        return nmi_;
    }

    void clear_nmi() {
        nmi_ = true;
    }

    [[nodiscard]] bool is_frame_complete() const {
        return frame_complete;
    }

    uint16_t bus_begin(uint8_t) override {
        return 0x2000;
    }

    uint16_t bus_end(uint8_t) override {
        return 0x2007;
    }

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

    void clock();

private:
    Canvas *screen;

    int cycle = 0;
    int scanline = 0;
    bool frame_complete = false;

    uint8_t dma_data_buffer = 0;
    bool dma_lsb_addr = false;  // first read is for MSB
    uint16_t dma_addr = 0;

    bool nmi_ = false;
};


#endif //MARIOBOX_P2C02_H
