//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_NES_H
#define MARIOBOX_NES_H

#include <cstdint>
#include <memory>
#include "C6502.h"
#include "Memory.h"
#include "P2C02.h"
#include "Cartridge.h"

class NES {
public:
    C6502 cpu;
    Memory cpu_ram;

    P2C02 ppu;
    Memory ppu_name_tables;
    Memory ppu_palettes;

    std::shared_ptr<Cartridge> cartridge = nullptr;

    NES();

    void reset() {
        cpu.reset();
        ticks = 0;
    }

    void clock() {
        if (ticks % 3 == 0) {
            cpu.clock();
        }
        ppu.clock();

        ticks += 1;
    }

//    func frame() -> Frame {
//        repeat {
//                clock();
//        } while !ppu.isFrameComplete();
//
//        return ppu.frame;
//    }

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

    void print_cpu_registers();

private:
    uint64_t ticks = 0;
};


#endif //MARIOBOX_NES_H
