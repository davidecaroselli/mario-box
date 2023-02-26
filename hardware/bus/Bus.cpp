//
// Created by Davide Caroselli on 25/02/23.
//

#include "Bus.h"

#include <utility>
#include <sstream>
#include "utils.h"

Bus::Bus() : ram(64 * 1024) {

}

void Bus::write(uint16_t addr, uint8_t data) {
    addr = validate_addr(addr);
    ram[addr] = data;
}

uint8_t Bus::read(uint16_t addr) {
    addr = validate_addr(addr);
    return ram[addr];
}

uint16_t Bus::validate_addr(uint16_t addr) {
    if (0x0000 <= addr && addr <= 0xFFFF) {
        return addr;
    } else {
        std::stringstream message;
        message << "invalid bus address at " << hexof(addr);
        throw bus_error(message.str());
    }
}

bus_error::bus_error(std::string message) : message(std::move(message)) {

}

const char *bus_error::what() const noexcept {
    return message.c_str();
}
