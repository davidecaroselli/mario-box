//
// Created by Davide Caroselli on 25/02/23.
//

#ifndef NESSY_BUS_H
#define NESSY_BUS_H

#include <cstdint>
#include <vector>
#include <string>

class Bus {
public:
    Bus();

public:
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

private:
    std::vector<uint8_t> ram;

    uint16_t validate_addr(uint16_t addr);
};

class bus_error: public std::exception {
public:
    explicit bus_error(std::string message);

    const char *what() const _NOEXCEPT override;

private:
    const std::string message;
};


#endif //NESSY_BUS_H
