//
// Created by Davide Caroselli on 04/03/23.
//

#include "ASM.h"
#include "core/C6502.h"
#include <sstream>
#include <iomanip>

using namespace std;

string hexof(uint16_t val, int len = 4) {
    stringstream num;
    num << "0x" << std::uppercase << std::setfill('0') << std::setw(len) << std::hex << val;
    return num.str();
}

ASM *ASM::decompile(Memory *prg, int begin, int end) {
    if (begin < 0) begin = prg->bus_begin(SystemBus::MAIN_BUS_ID);
    if (end < 0) end = prg->bus_end(SystemBus::MAIN_BUS_ID);

    map<uint16_t, string> lines;
    uint32_t addr = begin;
    while (addr <= end) {
        uint16_t entry = addr;
        uint8_t opcode = prg->bus_read(SystemBus::MAIN_BUS_ID, addr++);
        const inst_t &is = C6502::INSTRUCTIONS[opcode];

        if (is.name == "???") continue;

        stringstream line;
        line << hexof(entry) << ": " << is.name << "{" << is.addrmode.name << "} ";
        switch (is.addrmode.bytes) {
            case 0x00:
                break;
            case 0x01:
                line << hexof(prg->bus_read(SystemBus::MAIN_BUS_ID, addr++), 2);
                break;
            case 0x02: {
                uint16_t lo = prg->bus_read(SystemBus::MAIN_BUS_ID, addr++);
                uint16_t hi = prg->bus_read(SystemBus::MAIN_BUS_ID, addr++);
                line << hexof((hi << 8) | lo);
            }
                break;
            default:
                printf("unexpected address mode length %d", is.addrmode.bytes);
                break;
        }

        lines[entry] = line.str();
    }

    return new ASM(lines);
}

ASM::ASM(const map<uint16_t, string> &lines) : lines(lines) {}

vector<string> ASM::snippet(uint16_t center, int len) {
    vector<string> output(1 + len * 2);

    // forward
    auto it = lines.find(center);
    for (int i = 0; i <= len; i++) {
        if (it != lines.end()) {
            output[len + i] = it->second;
            it++;
        } else {
            break;
        }
    }

    // backward
    auto rit = lines.find(center);
    for (int i = 0; i <= len; i++) {
        if (rit != lines.end()) {
            output[len - i] = rit->second;
            rit--;
        } else {
            break;
        }
    }

    return output;
}