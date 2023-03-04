//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_MEMORY_H
#define MARIOBOX_MEMORY_H

#include "core/SystemBus.h"
#include "Mapper.h"

class Memory: public SBDevice {
public:
    Memory(uint32_t size, uint16_t begin, uint16_t end);

    Memory(std::vector<uint8_t> memory, std::shared_ptr<Mapper> mapper, uint16_t begin, uint16_t end);

    const std::vector<uint8_t> &data() {
        return data_;
    }

    uint16_t bus_begin(uint8_t bus_id) override {
        return begin;
    }

    uint16_t bus_end(uint8_t bus_id) override {
        return end;
    }

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

private:
    const uint16_t begin;
    const uint16_t end;
    const uint16_t mask;
    std::shared_ptr<Mapper> mapper;
    std::vector<uint8_t> data_;
};


#endif //MARIOBOX_MEMORY_H
