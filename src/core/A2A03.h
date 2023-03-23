//
// Created by Davide Caroselli on 22/03/23.
//

#ifndef MARIOBOX_A2A03_H
#define MARIOBOX_A2A03_H

#include "SystemBus.h"

class A2A03 : public SBDevice {
public:
    double demo_sw1_tone = 0;
    double demo_sw2_tone = 0;

    [[nodiscard]] double sample(double t) const;

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;
};


#endif //MARIOBOX_A2A03_H
