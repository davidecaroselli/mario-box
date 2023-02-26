//
// Created by Davide Caroselli on 25/02/23.
//

#ifndef NESSY_UTILS_H
#define NESSY_UTILS_H

#include <string>
#include <sstream>
#include <iomanip>

template<typename T>
std::string hexof(T i) {
    std::stringstream stream;
    stream << "0x"
           << std::setfill('0') << std::setw(sizeof(T) * 2)
           << std::hex << (uint64_t) i;
    return stream.str();
}

#endif //NESSY_UTILS_H
