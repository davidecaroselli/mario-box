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

class NES {
public:
    C6502 cpu;
    P2C02 ppu;

    std::shared_ptr<Cartridge> cartridge = nullptr;

    NES();

    void reset() {
        cpu.reset();
        ticks = 0;
    }

    void clock();

    Frame *next_frame() {
        do {
            clock();
        } while (!ppu.is_frame_complete());

        return ppu.frame();
    }

    void insert(Cartridge *crt) {
        if (this->cartridge) eject();

        this->cpu.bus.connect(&crt->prg);
        this->ppu.bus.connect(&crt->chr);
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
