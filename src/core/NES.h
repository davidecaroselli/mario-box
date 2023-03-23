//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_NES_H
#define MARIOBOX_NES_H

#include <cstdint>
#include <memory>
#include "C6502.h"
#include "Memory.h"
#include "core/ppu/P2C02.h"
#include "Cartridge.h"
#include "Controllers.h"
#include "DMA.h"
#include "A2A03.h"

class NES {
public:
    C6502 cpu;
    A2A03 apu;
    P2C02 ppu;
    DMA dma;
    Controllers controllers;

    std::shared_ptr<Cartridge> cartridge = nullptr;

    explicit NES(Canvas *screen);

    void reset();

    void frame() {
        do {
            clock();
        } while (!ppu.is_frame_complete());
    }

    void clock();

    void insert(Cartridge *crt) {
        if (this->cartridge) eject();

        this->cpu.bus.connect(0x4020, 0xFFFF, &crt->prg);
        this->ppu.bus.connect(0x0000, 0x1FFF, &crt->chr);
        this->ppu.name_tables.mirroring = crt->mirroring;
        this->cartridge = std::shared_ptr<Cartridge>(crt);

        reset();
    }

    void eject() {
        if (this->cartridge) {
            this->cpu.bus.disconnect(&cartridge->prg);
            this->ppu.bus.disconnect(&cartridge->chr);
            this->cartridge = nullptr;
        }
    }

    // - Debug utilities -----------------------------------------------------------------------------------------------

    void step();

private:
    uint64_t ticks = 0;
};


#endif //MARIOBOX_NES_H
