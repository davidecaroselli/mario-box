//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_ASM_H
#define MARIOBOX_ASM_H

#include <map>
#include <string>
#include "core/Memory.h"

class ASM {
public:
    static ASM *decompile(Memory *prg, uint32_t begin = 0x0000, uint32_t end = 0x1FFF);

    std::vector<std::string> snippet(uint16_t center, int len = 8);

private:
    std::map<uint16_t, std::string> lines;

public:
    explicit ASM(const std::map<uint16_t, std::string> &lines);
};


#endif //MARIOBOX_ASM_H
