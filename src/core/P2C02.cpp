//
// Created by Davide Caroselli on 04/03/23.
//

#include "P2C02.h"

P2C02::P2C02(): bus(SystemBus::PPU_BUS_ID) {
}

uint8_t P2C02::bus_read(uint8_t bus_id, uint16_t addr) {
    return 0;
}

void P2C02::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {

}

void P2C02::clock() {

}
