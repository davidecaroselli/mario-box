//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_P2C02_H
#define MARIOBOX_P2C02_H

#include "SystemBus.h"

class P2C02: public SBDevice {
public:
    SystemBus bus;

    P2C02();

    uint16_t bus_begin(uint8_t) override {
        return 0x2000;
    }

    uint16_t bus_end(uint8_t) override {
        return 0x2007;
    }

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

    void clock();
};


#endif //MARIOBOX_P2C02_H
