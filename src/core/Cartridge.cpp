//
// Created by Davide Caroselli on 04/03/23.
//

#include "Cartridge.h"

#include <utility>
#include <fstream>
#include <iostream>

using namespace std;

struct header_t {
    char name[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t prg_rom_size = 0;
    uint8_t chr_rom_size = 0;
    uint8_t mapper_flags_6 = 0;
    uint8_t mapper_flags_7 = 0;
    uint8_t prg_ram_size = 0;
    uint8_t tv_system_0 = 0;
    uint8_t tv_system_1 = 0;
    char _[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
};

Cartridge::Cartridge(const Mirroring mirroring, Mapper *mapper, const std::vector<uint8_t> &prg,
                     const std::vector<uint8_t> &chr)
        : mirroring(mirroring), mapper(mapper),
          prg(prg, this->mapper, 0x4020, 0xFFFF),
          chr(chr, this->mapper, 0x0000, 0x1FFF) {}

Cartridge *Cartridge::load(const string &path) {
    ifstream ifs(path, ios::in | ios::binary);
    if (!ifs.is_open())
        return nullptr;

    // read header
    header_t header;
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header_t));

    if (string(header.name, 4) != "NES\x1A")
        return nullptr;


    bool has_trainer = (header.mapper_flags_6 & 0x04) != 0;
    Mirroring mirroring = (header.mapper_flags_6 & 0x01) != 0 ? Vertical : Horizontal;
    uint8_t mapper_id = (header.mapper_flags_7 & 0xF0) | (header.mapper_flags_7 >> 4);

    Mapper *mapper = Mapper::create(mapper_id, header.prg_rom_size, header.chr_rom_size);

    // read body
    if (has_trainer)
        ifs.seekg(512, std::ios_base::cur);

    vector<uint8_t> prg_memory(header.prg_rom_size * 16384);
    ifs.read((char *) prg_memory.data(), (int) prg_memory.size());

    vector<uint8_t> chr_memory(header.chr_rom_size * 8192);
    ifs.read((char *) chr_memory.data(), (int) chr_memory.size());

    return new Cartridge(mirroring, mapper, prg_memory, chr_memory);
}
