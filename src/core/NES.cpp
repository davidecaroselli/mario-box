//
// Created by Davide Caroselli on 04/03/23.
//

#include "NES.h"
#include "olcPixelGameEngine.h"

NES::NES(Canvas *screen) : ppu(screen), dma(&cpu, &ppu) {
    cpu.bus.connect(0x2000, 0x2007, &ppu);
    cpu.bus.connect(0x4000, 0x4013, &apu);
    cpu.bus.connect(0x4014, 0x4014, &dma);
    cpu.bus.connect(0x4015, 0x4015, &apu);
    cpu.bus.connect(0x4016, 0x4016, &controllers);
    cpu.bus.connect(0x4017, 0x4017, &apu);
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

    if (ticks % 6 == 0)
        apu.clock();

    ppu.clock();

    if (ppu.nmi()) {
        ppu.clear_nmi();
        cpu.nmi();
    }

    ticks += 1;
}