//
// Created by Davide Caroselli on 04/03/23.
//

#include "ASM.h"
#include "core/C6502.h"
#include <sstream>
#include <iomanip>

using namespace std;

string hexof(uint16_t val, int len = 4, bool prefix = true) {
    stringstream num;
    if (prefix) num << "0x";
    num << std::uppercase << std::setfill('0') << std::setw(len) << std::hex << val;
    return num.str();
}

string addrmode_str(const string &addrmode, uint16_t addr, uint8_t bytes) {
    stringstream out;

    if (bytes > 0) {
        if (addrmode == "ABS" || addrmode == "ABX" || addrmode == "ABY") out << "$";
        if (addrmode == "ZP0" || addrmode == "ZPX" || addrmode == "ZPY") out << "$";
        if (addrmode == "IMM") out << "#";
        if (addrmode == "IND" || addrmode == "IZX" || addrmode == "IZY") out << "($";

        // print number
        if (addrmode == "REL") {
            if (addr & 0x80)
                out << "-" << (int) (~addr & 0x007F) - 1;
            else
                out << "+" << (int) addr;
        } else {
            out << hexof(addr, bytes * 2, false);
        }

        if (addrmode == "ABX" || addrmode == "ZPX") out << ",X";
        if (addrmode == "ABY" || addrmode == "ZPY") out << ",Y";
        if (addrmode == "IND") out << ")";
        if (addrmode == "IZX") out << ",X)";
        if (addrmode == "IZY") out << "),Y";
    }

    stringstream pad_out;
    pad_out << std::left << std::setfill(' ') << std::setw(9) << out.str();
    return pad_out.str();
}

ASM *ASM::decompile(Memory *prg, uint32_t begin, uint32_t end) {
    map<uint16_t, string> lines;
    uint32_t addr = begin;
    while (addr <= end) {
        uint16_t entry = addr;
        uint8_t opcode = prg->bus_read(SystemBus::MAIN_BUS_ID, addr++);
        const inst_t &is = C6502::INSTRUCTIONS[opcode];

        if (is.name == "???") continue;

        stringstream line;
        line << hexof(entry) << ": " << is.name << " ";

        switch (is.addrmode.bytes) {
            case 0:
                line << addrmode_str(is.addrmode.name, 0, 0);
                break;
            case 1: {
                uint8_t val = prg->bus_read(SystemBus::MAIN_BUS_ID, addr++);
                line << addrmode_str(is.addrmode.name, val, is.addrmode.bytes);
            }
                break;
            case 2: {
                uint16_t lo = prg->bus_read(SystemBus::MAIN_BUS_ID, addr++);
                uint16_t hi = prg->bus_read(SystemBus::MAIN_BUS_ID, addr++);
                line << addrmode_str(is.addrmode.name, (hi << 8) | lo, is.addrmode.bytes);
            }
                break;
            default:
                printf("unexpected address mode length %d", is.addrmode.bytes);
                break;
        }

        line << "  (" << is.addrmode.name << ")";
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