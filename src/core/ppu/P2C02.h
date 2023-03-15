//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_P2C02_H
#define MARIOBOX_P2C02_H

#include "core/SystemBus.h"
#include "Palettes.h"
#include "core/C6502.h"
#include "platform/Canvas.h"
#include "NameTables.h"

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
        uint8_t nametable_h: 1;
        uint8_t nametable_v: 1;
        uint8_t increment_mode: 1;
        uint8_t pattern_sprite: 1;
        uint8_t pattern_background: 1;
        uint8_t sprite_size: 1;
        uint8_t slave_mode: 1;
        uint8_t enable_nmi: 1;
    };
    uint8_t reg = 0;
};

union oam_attr_t {
    struct {
        uint8_t palette: 2;
        uint8_t _: 3;
        uint8_t priority: 1;
        uint8_t flip_h: 1;
        uint8_t flip_v: 1;
    };
    uint8_t val;
};

struct oam_t {
    uint8_t y;
    uint8_t id;
    oam_attr_t attr;
    uint8_t x;
};

class P2C02 : public SBDevice {
public:
    SystemBus bus;
    Palettes palettes;
    NameTables name_tables;

    ppustatus_t status;
    ppumask_t mask;
    ppucontrol_t control;

    oam_t sprites[64] = {};
    uint8_t *sprites_ptr = (uint8_t *) &sprites;

    explicit P2C02(Canvas *screen);

    [[nodiscard]] bool nmi() const {
        return nmi_;
    }

    void clear_nmi() {
        nmi_ = false;
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

    union ppu_reg_t {
        struct {
            uint16_t coarse_x: 5;
            uint16_t coarse_y: 5;
            uint8_t nametable_h: 1;
            uint8_t nametable_v: 1;
            uint16_t fine_y: 3;
            uint16_t _: 1;
        };

        uint16_t reg;
    };

    bool odd_frame = false;
    bool first_write_toggle = true;
    ppu_reg_t vram_addr = {0x0000};
    ppu_reg_t t_vram_addr = {0x0000};
    uint8_t fine_x = 0x00;
    uint8_t ppu_data_buffer = 0;

    void scroll();

    bool nmi_ = false;

    // latch registers
    uint16_t lr_tile = 0;
    uint16_t lr_tile_attr = 0;
    uint16_t lr_chr_lsb = 0;
    uint16_t lr_chr_msb = 0;

    // shift registers
    uint16_t sl_chr_lsb = 0;
    uint16_t sl_chr_msb = 0;
    uint16_t sl_chr_attr_lsb = 0;
    uint16_t sl_chr_attr_msb = 0;

    void render_background(uint8_t *out_pixel, uint8_t *out_palette);

    uint8_t read_tile();

    uint8_t read_tile_attr();

    uint8_t read_pattern(bool lsb);

    // sprites

    uint8_t oam_addr = 0;
    oam_t visible_sprites[8] = {};
    uint8_t visible_sprites_count = 0;
    uint8_t sr_vsprite_lsb[8] = {};
    uint8_t sr_vsprite_msb[8] = {};

    bool sprite_zero_visible = false;
    bool sprite_zero_rendering = false;

    [[nodiscard]] inline uint8_t get_sprite_size() const {
        return control.sprite_size == 1 ? 16 : 8;
    }

    void render_sprites(uint8_t *out_pixel, uint8_t *out_palette, uint8_t *out_priority);
};


#endif //MARIOBOX_P2C02_H
