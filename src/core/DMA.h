//
// Created by Davide Caroselli on 12/03/23.
//

#ifndef MARIOBOX_DMA_H
#define MARIOBOX_DMA_H


#include "SystemBus.h"
#include "C6502.h"
#include "core/ppu/P2C02.h"

class DMA : public SBDevice {
public:
    DMA(C6502 *cpu, P2C02 *ppu);

    [[nodiscard]] bool is_running() const {
        return active;
    }

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override {
        return 0x00;
    }

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

    void clock(uint64_t ticks);

private:
    bool active = false;
    bool sync = true;
    uint16_t page = 0;
    uint8_t address = 0;
    uint8_t data = 0;

    C6502 *cpu;
    P2C02 *ppu;
};


#endif //MARIOBOX_DMA_H
