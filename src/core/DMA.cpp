//
// Created by Davide Caroselli on 12/03/23.
//

#include "DMA.h"

DMA::DMA(C6502 *cpu, P2C02 *ppu): cpu(cpu), ppu(ppu) {
}

void DMA::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    page = ((uint16_t) val) << 8;
    address = 0;
    active = true;
}

#include <iostream>

void DMA::clock(uint64_t ticks) {
    if (sync) {
        if (ticks % 2 == 1) {
            sync = false;
        }
    } else {
        if (ticks % 2 == 0) {
            data = cpu->bus.read(page | address);
        } else {
            ppu->sprites_ptr[address] = data;
            address++;

            if (address == 0) {
                // reset
                active = false;
                sync = true;
            }
        }
    }
}
