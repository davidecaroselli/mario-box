//
// Created by Davide Caroselli on 04/03/23.
//

#include "Memory.h"

#include <utility>

Memory::Memory(uint32_t size, uint16_t begin, uint16_t end) : begin(begin), end(end), mask(size - 1), data_(size) {
}

uint8_t Memory::bus_read(uint8_t bus_id, uint16_t addr) {
    if (data_.empty()) return 0x00;
    uint16_t idx = this->mapper ? this->mapper->remap(bus_id, addr) : ((addr - begin) & mask);
    return data_[idx];
}

void Memory::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    if (!data_.empty()) {
        uint16_t idx = this->mapper ? this->mapper->remap(bus_id, addr) : ((addr - begin) & mask);
        data_[idx] = val;
    }
}

Memory::Memory(std::vector<uint8_t> memory, std::shared_ptr<Mapper> mapper, uint16_t begin, uint16_t end)
        : begin(begin), end(end), mask(0), mapper(std::move(mapper)), data_(std::move(memory)) {
}
