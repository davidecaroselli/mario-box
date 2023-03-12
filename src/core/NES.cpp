//
// Created by Davide Caroselli on 04/03/23.
//

#include "NES.h"
#include "olcPixelGameEngine.h"

NES::NES(Canvas *screen) : ppu(screen), dma(&cpu, &ppu) {
    cpu.bus.connect(&ppu);
    cpu.bus.connect(&dma);
    cpu.bus.connect(&controllers);
}

void NES::reset() {
    cpu.reset();
    ticks = 0;
}

void NES::step() {
    while (cpu.cycles() != 0) {
        clock();
    }
    clock();
    clock();
    clock();
}

void NES::clock() {
    if (ticks % 3 == 0) {
        if (dma.is_running()) {
            dma.clock(ticks);
        } else {
            cpu.clock();
        }
    }
    ppu.clock();

    if (ppu.nmi()) {
        ppu.clear_nmi();
        cpu.nmi();
    }

    ticks += 1;
}