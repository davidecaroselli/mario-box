//
// Created by Davide Caroselli on 07/03/23.
//

#include "PatternTables.h"

using namespace std;

PatternTables::PatternTables(const vector<uint8_t> &memory, const shared_ptr<Mapper> &mapper)
        : Memory(memory, mapper, 0x0000, 0x1FFF) {}

void PatternTables::render(uint8_t table, const Palette &palette, Canvas *canvas) {
    assert(canvas->width == 128);
    assert(canvas->height == 128);
    assert(table < 2);

    uint16_t table_mask = ((uint16_t)table) << 12;

    for (uint16_t row = 0; row < 16; ++row) {
        for (uint16_t col = 0; col < 16; ++col) {
            uint16_t addr = table_mask | (row << 8) | (col << 4);

            for (uint16_t tile_row = 0; tile_row < 8; ++tile_row) {
                uint16_t tile_addr = addr | tile_row;
                uint8_t bp0 = this->bus_read(SystemBus::PPU_BUS_ID, tile_addr);
                tile_addr |= (1 << 3);
                uint8_t bp1 = this->bus_read(SystemBus::PPU_BUS_ID, tile_addr);

                for (uint8_t mask = 0; mask < 8; ++mask) {
                    uint8_t bit0 = (bp0 >> mask) & 0x01;
                    uint8_t bit1 = (bp1 >> mask) & 0x01;
                    uint8_t color_idx = (bit1 << 1) | bit0;
                    canvas->set((col << 3) + (7 - mask), (row << 3) + tile_row, palette.colors[color_idx]);
                }
            }
        }
    }
}

void PatternTables::render_tile(uint8_t table, uint8_t tile, const Palette &palette, Canvas *canvas, uint8_t x, uint8_t y) {
    uint16_t table_mask = ((uint16_t)table) << 12;
    uint16_t addr = table_mask | (tile << 4);

    for (uint16_t tile_row = 0; tile_row < 8; ++tile_row) {
        uint16_t tile_addr = addr | tile_row;
        uint8_t bp0 = this->bus_read(SystemBus::PPU_BUS_ID, tile_addr);
        tile_addr |= (1 << 3);
        uint8_t bp1 = this->bus_read(SystemBus::PPU_BUS_ID, tile_addr);

        for (uint8_t mask = 0; mask < 8; ++mask) {
            uint8_t bit0 = (bp0 >> mask) & 0x01;
            uint8_t bit1 = (bp1 >> mask) & 0x01;
            uint8_t color_idx = (bit1 << 1) | bit0;
            canvas->set(x + (7 - mask), y + tile_row, palette.colors[color_idx]);
        }
    }
}
