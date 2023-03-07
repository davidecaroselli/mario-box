//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_CARTRIDGE_H
#define MARIOBOX_CARTRIDGE_H

#include "Mapper.h"
#include "Memory.h"
#include "core/ppu/PatternTables.h"

enum Mirroring {
    Vertical,
    Horizontal
};

class Cartridge {
public:
    const Mirroring mirroring;
    const std::shared_ptr<Mapper> mapper;
    Memory prg;
    PatternTables chr;

    static Cartridge *load(const std::string &path);

private:
    Cartridge(Mirroring mirroring, Mapper *mapper, const std::vector<uint8_t> &prg, const std::vector<uint8_t> &chr);
};


#endif //MARIOBOX_CARTRIDGE_H
