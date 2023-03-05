//
// Created by Davide Caroselli on 04/03/23.
//

#include "Mapper.h"
#include "Logger.h"

class Mapper000: public Mapper {
public:
    explicit Mapper000(const uint8_t prg_banks) : prg_banks(prg_banks) {}

    uint32_t remap(uint8_t bus_id, uint16_t addr) override {
        if (bus_id == SystemBus::PPU_BUS_ID)
            return addr;
        return prg_banks > 1 ? addr & 0x7FFF : addr & 0x3FFF;
    }

private:
    const uint8_t prg_banks;
};

Mapper *Mapper::create(uint8_t type, uint8_t prg_banks, uint8_t chr_banks) {
    switch (type) {
        case 0x00:
            return new Mapper000(prg_banks);
        default:
            Logger::err("unsupported mapper type %d", type);
            exit(2);
    }
}
