//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_MAPPER_H
#define MARIOBOX_MAPPER_H


#include <cstdint>
#include "SystemBus.h"

class Mapper {
public:
    static Mapper *create(uint8_t type, uint8_t prg_banks, uint8_t chr_banks);

    virtual uint32_t remap(uint8_t bus_id, uint16_t addr) = 0;
};


#endif //MARIOBOX_MAPPER_H
