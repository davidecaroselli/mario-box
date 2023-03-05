//
// Created by Davide Caroselli on 04/03/23.
//

#include "NES.h"

NES::NES(){
    cpu.bus.connect(&ppu);
}

void NES::step() {
    while (cpu.cycles() != 0) {
        clock();
    }
    clock();
    clock();
    clock();
}

void NES::clock()  {
    if (ticks % 3 == 0) {
        cpu.clock();
    }
    ppu.clock();

    if (ppu.nmi()) {
        ppu.clear_nmi();
        cpu.nmi();
    }

    ticks += 1;
}