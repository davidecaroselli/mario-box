//
// Created by Davide Caroselli on 22/03/23.
//

#ifndef MARIOBOX_A2A03_H
#define MARIOBOX_A2A03_H

#include "SystemBus.h"

class A2A03 : public SBDevice {
public:
    double sample(double t);

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;
};


#endif //MARIOBOX_A2A03_H
