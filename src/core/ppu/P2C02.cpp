//
// Created by Davide Caroselli on 04/03/23.
//

#include "P2C02.h"
#include "Logger.h"

// https://stackoverflow.com/a/2602885
uint8_t flip_horizontal(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

P2C02::P2C02(Canvas *screen) : bus(SystemBus::PPU_BUS_ID), screen(screen) {
    assert(screen->width == 256);
    assert(screen->height == 240);

    bus.connect(&name_tables);
    bus.connect(&palettes);
}

uint8_t P2C02::bus_read(uint8_t bus_id, uint16_t addr) {
    uint8_t data = 0;

    if (bus_id == SystemBus::MAIN_BUS_ID) {
        switch (addr) {
            case 0x2000:  // control
            case 0x2001: // mask
                break;
            case 0x2002: // status
                data = (status.reg & 0xE0) | (ppu_data_buffer & 0x1F); // simulating dirty buffer on unused status bits
                status.vertical_blank = 0;
                first_write_toggle = true;
                break;
            case 0x2003: // OAM address
                break;
            case 0x2004: // OAM data
                data = sprites_ptr[oam_addr];
                break;
            case 0x2005: // scroll
                break;
            case 0x2006: // VRAM addr
                Logger::err("attempting to read from PPU DMA address");
                break;
            case 0x2007: // VRAM data
                data = ppu_data_buffer;
                ppu_data_buffer = bus.read(vram_addr.reg);
                if (vram_addr.reg > 0x3F00) data = ppu_data_buffer; // no buffered read for palette
                vram_addr.reg += control.increment_mode ? 32 : 1;
                break;
            default:
                Logger::err("invalid PPU address 0x%04X", addr);
                break;
        }
    }

    return data;
}

void P2C02::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    if (bus_id == SystemBus::MAIN_BUS_ID) {
        switch (addr) {
            case 0x2000:  // control
                control.reg = val;
                t_vram_addr.nametable_h = control.nametable_h;
                t_vram_addr.nametable_v = control.nametable_v;
                break;
            case 0x2001: // mask
                mask.reg = val;
                break;
            case 0x2002: // status
                Logger::warn("(PPU) attempting to write to PPU status");
                break;
            case 0x2003: // OAM address
                oam_addr = val;
                break;
            case 0x2004: // OAM data
                sprites_ptr[oam_addr++] = val;
                break;
            case 0x2005: // scroll
                if (first_write_toggle) {
                    t_vram_addr.coarse_x = (val >> 3) & 0x1F;
                    fine_x = val & 0x07;
                } else {
                    t_vram_addr.coarse_y = (val >> 3) & 0x1F;
                    t_vram_addr.fine_y = val & 0x07;
                }

                first_write_toggle = !first_write_toggle;
                break;
            case 0x2006: // VRAM addr
                if (first_write_toggle) {
                    t_vram_addr.reg = (t_vram_addr.reg & 0x00FF) | ((uint16_t) (val & 0x3F) << 8);
                } else {
                    t_vram_addr.reg = (t_vram_addr.reg & 0xFF00) | (uint16_t) val;
                    vram_addr.reg = t_vram_addr.reg;
                }

                first_write_toggle = !first_write_toggle;
                break;
            case 0x2007: // VRAM data
                bus.write(vram_addr.reg, val);
                vram_addr.reg += control.increment_mode ? 32 : 1;
                break;
            default:
                Logger::err("invalid PPU address 0x%04X", addr);
                break;
        }
    }
}

void P2C02::clock() {
    frame_complete = false;

    if (scanline == -1 && cycle == 1) {
        status.vertical_blank = 0;
        status.sprite_overflow = 0;
        status.sprite_zero_hit = 0;
    }

    scroll();

    uint8_t bg_pixel = 0, bg_palette = 0;
    render_background(&bg_pixel, &bg_palette);

    uint8_t fg_pixel = 0, fg_palette = 0, fg_priority = 0;
    render_sprites(&fg_pixel, &fg_palette, &fg_priority);

    if (0 <= scanline && scanline <= 239 &&
        1 <= cycle && cycle <= 256) {

        uint8_t pixel = 0, palette = 0;
        if (bg_pixel == 0 && fg_pixel > 0) {
            pixel = fg_pixel;
            palette = fg_palette;
        } else if (bg_pixel > 0 && fg_pixel == 0) {
            pixel = bg_pixel;
            palette = bg_palette;
        } else if (bg_pixel > 0 && fg_pixel > 0) {
            if (fg_priority) {
                pixel = fg_pixel;
                palette = fg_palette;
            } else {
                pixel = bg_pixel;
                palette = bg_palette;
            }

            if (sprite_zero_visible && sprite_zero_rendering) {
                if (mask.render_background && mask.render_sprites) {
                    if (~(mask.render_background_left | mask.render_sprites_left)) {
                        if (9 <= cycle && cycle <= 256)
                            status.sprite_zero_hit = 1;
                    } else {
                        if (1 <= cycle && cycle <= 256)
                            status.sprite_zero_hit = 1;
                    }
                }
            }
        }

        screen->set(cycle - 1, scanline, this->palettes.color_at(palette, pixel));
    }

    if (scanline == 241 && cycle == 1) {
        status.vertical_blank = 1;
        if (control.enable_nmi) nmi_ = true;
    }

    if (-1 <= scanline && scanline < 240 && 257 <= cycle && cycle <= 320) {
        oam_addr = 0;
    }

    // - Loop automation -----------------------------------------------------------------------------------------------

    cycle++;

    if (cycle >= 341) {
        cycle = 0;
        scanline += 1;

        if (scanline >= 261) {
            scanline = -1;
            frame_complete = true;
        }
    }

    if (scanline == 0 && cycle == 0) {
        if (odd_frame) cycle = 1;  // skip first cycle in odd frames
        odd_frame = !odd_frame;
    }
}

uint8_t P2C02::read_tile() {
    return bus.read(0x2000 | (vram_addr.reg & 0x0FFF));
}

uint8_t P2C02::read_tile_attr() {
    uint8_t val = bus.read(0x23C0
                           | (vram_addr.reg & 0x0C00)  // nametable
                           | ((vram_addr.reg >> 4) & 0x38)  // coarse y
                           | ((vram_addr.reg >> 2) & 0x07));  // coarse x

    // attrib = (bottom_right << 6) | (bottom_left << 4) | (top_right << 2) | (top_left << 0)
    if (vram_addr.coarse_y & 0x02) val >>= 4;
    if (vram_addr.coarse_x & 0x02) val >>= 2;
    return val & 0x03;
}

uint8_t P2C02::read_pattern(bool lsb) {
    uint16_t addr = (control.pattern_background << 12) | (lr_tile << 4) | vram_addr.fine_y;
    if (!lsb) addr += 8;
    return bus.read(addr);
}

void P2C02::scroll() {
    if (!mask.render_background && !mask.render_sprites) return;
    if (scanline > 239) return;

    if (cycle == 256) {  // vertical increment
        if (vram_addr.fine_y < 7) {
            vram_addr.fine_y++;
        } else {
            vram_addr.fine_y = 0;
            if (vram_addr.coarse_y == 29) {
                vram_addr.coarse_y = 0;
                vram_addr.nametable_v = ~vram_addr.nametable_v;
            } else if (vram_addr.coarse_y == 31) {
                vram_addr.coarse_y = 0;
            } else {
                vram_addr.coarse_y++;
            }
        }
    } else if (cycle == 257) {  // copy horizontal position from t to v
        vram_addr.coarse_x = t_vram_addr.coarse_x;
        vram_addr.nametable_h = t_vram_addr.nametable_h;
    } else if (280 <= cycle && cycle <= 304 && scanline == -1) {  // repeatedly copy the vertical bits from t to v
        vram_addr.coarse_y = t_vram_addr.coarse_y;
        vram_addr.nametable_v = t_vram_addr.nametable_v;
        vram_addr.fine_y = t_vram_addr.fine_y;
    }

    if ((cycle >= 328 || cycle <= 256) && cycle != 0 && cycle % 8 == 0) {  //  increment the horizontal position in v
        if (vram_addr.coarse_x == 31) {
            vram_addr.coarse_x = 0;
            vram_addr.nametable_h = ~vram_addr.nametable_h;
        } else {
            vram_addr.coarse_x++;
        }
    }
}

void P2C02::render_background(uint8_t *out_pixel, uint8_t *out_palette) {
    if (-1 <= scanline && scanline < 240) {  // visible scan-lines and pre-render scanline

        // The shifters are reloaded during ticks 9, 17, 25, ..., 257.
        if (cycle > 1 && (cycle - 1) % 8 == 1) {
            // load background shift registers
            sl_chr_lsb = (sl_chr_lsb & 0xFF00) | lr_chr_lsb;
            sl_chr_msb = (sl_chr_msb & 0xFF00) | lr_chr_msb;
            sl_chr_attr_lsb = (sl_chr_attr_lsb & 0xFF00) | ((lr_tile_attr & 0x01) ? 0xFF : 0x00);
            sl_chr_attr_msb = (sl_chr_attr_msb & 0xFF00) | ((lr_tile_attr & 0x02) ? 0xFF : 0x00);
        }

        // read latch registers
        if (1 <= cycle && cycle <= 337) {

            // update shift registers
            if (cycle > 1 && mask.render_background) {
                sl_chr_lsb <<= 1;
                sl_chr_msb <<= 1;
                sl_chr_attr_lsb <<= 1;
                sl_chr_attr_msb <<= 1;
            }

            switch ((cycle - 1) % 8) {
                case 0:
                    lr_tile = read_tile();
                    break;
                case 2:
                    lr_tile_attr = read_tile_attr();
                    break;
                case 4:
                    lr_chr_lsb = read_pattern(true);
                    break;
                case 6:
                    lr_chr_msb = read_pattern(false);
                    break;
            }
        } else if (cycle == 338 || cycle == 340) {
            lr_tile = read_tile();
        }
    }

    // put pixel on screen
    if (0 <= scanline && scanline < 240 &&
        1 <= cycle && cycle <= 256) {

        if (mask.render_background) {
            uint16_t bitmask = 0x8000 >> fine_x;

            uint8_t bit0 = (sl_chr_lsb & bitmask) ? 0x01 : 0x00;
            uint8_t bit1 = (sl_chr_msb & bitmask) ? 0x01 : 0x00;
            *out_pixel = (bit1 << 1) | bit0;

            uint8_t pal_bit0 = (sl_chr_attr_lsb & bitmask) ? 0x01 : 0x00;
            uint8_t pal_bit1 = (sl_chr_attr_msb & bitmask) ? 0x01 : 0x00;
            *out_palette = (pal_bit1 << 1) | pal_bit0;
        }
    }
}

void P2C02::render_sprites(uint8_t *out_pixel, uint8_t *out_palette, uint8_t *out_priority) {
    // Calculate sprites for next scanline
    if (-1 <= scanline && scanline < 239) {

        if (cycle == 257) {
            // reset
            std::memset(visible_sprites, 0xFF, 8 * sizeof(oam_t));
            visible_sprites_count = 0;
            sprite_zero_visible = false;

            // search visible sprites for next scanline
            for (int i = 0; i < 64; i++) {
                auto &sprite = sprites[i];

                int diff = scanline - (int) sprite.y;
                if (0 <= diff && diff < get_sprite_size()) {
                    if (visible_sprites_count < 8) {
                        if (i == 0) sprite_zero_visible = true;

                        visible_sprites[visible_sprites_count] = sprite;
                        visible_sprites_count++;
                    } else {
                        status.sprite_overflow = 1;
                        break;
                    }
                }
            }
        } else if (cycle == 340) {
            // load shift registers
            for (int i = 0; i < visible_sprites_count; i++) {
                oam_t &sprite = visible_sprites[i];

                uint8_t pattern_lsb, pattern_msb;
                uint16_t pattern_addr;
                if (control.sprite_size == 0) {  // 8x8
                    if (sprite.attr.flip_v) {
                        pattern_addr = (control.pattern_sprite << 12)
                                       | (sprite.id << 4)
                                       | (7 - (scanline - sprite.y));
                    } else {
                        pattern_addr = (control.pattern_sprite << 12)
                                       | (sprite.id << 4)
                                       | (scanline - sprite.y);
                    }
                } else {  // 16x8
                    //TODO
                    exit(1);
                }

                pattern_lsb = bus.read(pattern_addr);
                pattern_msb = bus.read(pattern_addr + 8);

                if (sprite.attr.flip_h) {
                    pattern_lsb = flip_horizontal(pattern_lsb);
                    pattern_msb = flip_horizontal(pattern_msb);
                }

                sr_vsprite_lsb[i] = pattern_lsb;
                sr_vsprite_msb[i] = pattern_msb;
            }
        }
    }

    // render sprite
    if (0 <= scanline && scanline <= 239 &&
        1 <= cycle && cycle <= 256) {

        // update shift registers
        if (cycle > 1) {
            for (int i = 0; i < visible_sprites_count; i++) {
                oam_t &sprite = visible_sprites[i];
                if (sprite.x > 0) {
                    sprite.x--;
                } else {
                    sr_vsprite_lsb[i] <<= 1;
                    sr_vsprite_msb[i] <<= 1;
                }
            }
        }

        if (mask.render_sprites) {
            sprite_zero_rendering = false;

            for (int i = 0; i < visible_sprites_count; i++) {
                oam_t &sprite = visible_sprites[i];

                if (sprite.x == 0) {
                    uint8_t bit0 = (sr_vsprite_lsb[i] & 0x80) ? 0x01 : 0x00;
                    uint8_t bit1 = (sr_vsprite_msb[i] & 0x80) ? 0x01 : 0x00;
                    *out_pixel = (bit1 << 1) | bit0;

                    *out_palette = sprite.attr.palette + 0x04;
                    *out_priority = sprite.attr.priority ? 0 : 1;

                    if (*out_pixel != 0) {
                        if (i == 0) sprite_zero_rendering = true;
                        break;
                    }
                }
            }
        }
    }
}
