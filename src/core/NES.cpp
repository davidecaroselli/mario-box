//
// Created by Davide Caroselli on 04/03/23.
//

#include "NES.h"

NES::NES()
        : cpu_ram(2048, 0x0000, 0x1FFF),
          ppu_name_tables(2048, 0x2000, 0x2FFF),
          ppu_palettes(0xFF, 0x3F00, 0x3FFF) {
    cpu.bus.connect(&cpu_ram);

    ppu.bus.connect(&ppu_name_tables);
    ppu.bus.connect(&ppu_palettes);

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