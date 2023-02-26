//
// Created by Davide Caroselli on 25/02/23.
//

#include "C6502.h"

#include <utility>
#include <iostream>
#include "utils.h"

#define is_same_page(pa, pb) (((pa) & 0xFF00) == ((pb) & 0xFF00))

C6502::C6502(std::shared_ptr<Bus> bus) : bus(std::move(bus)) {
    instructions = {
            {"BRK", &C6502::BRK, &C6502::IMM, 7},
            {"ORA", &C6502::ORA, &C6502::IZX, 6},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 3},
            {"ORA", &C6502::ORA, &C6502::ZP0, 3},
            {"ASL", &C6502::ASL, &C6502::ZP0, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"PHP", &C6502::PHP, &C6502::IMP, 3},
            {"ORA", &C6502::ORA, &C6502::IMM, 2},
            {"ASL", &C6502::ASL, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"ORA", &C6502::ORA, &C6502::ABS, 4},
            {"ASL", &C6502::ASL, &C6502::ABS, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"BPL", &C6502::BPL, &C6502::REL, 2},
            {"ORA", &C6502::ORA, &C6502::IZY, 5},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"ORA", &C6502::ORA, &C6502::ZPX, 4},
            {"ASL", &C6502::ASL, &C6502::ZPX, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"CLC", &C6502::CLC, &C6502::IMP, 2},
            {"ORA", &C6502::ORA, &C6502::ABY, 4},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 7},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"ORA", &C6502::ORA, &C6502::ABX, 4},
            {"ASL", &C6502::ASL, &C6502::ABX, 7},
            {"???", nullptr,     &C6502::IMP, 7},
            {"JSR", &C6502::JSR, &C6502::ABS, 6},
            {"AND", &C6502::AND, &C6502::IZX, 6},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"BIT", &C6502::BIT, &C6502::ZP0, 3},
            {"AND", &C6502::AND, &C6502::ZP0, 3},
            {"ROL", &C6502::ROL, &C6502::ZP0, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"PLP", &C6502::PLP, &C6502::IMP, 4},
            {"AND", &C6502::AND, &C6502::IMM, 2},
            {"ROL", &C6502::ROL, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 2},
            {"BIT", &C6502::BIT, &C6502::ABS, 4},
            {"AND", &C6502::AND, &C6502::ABS, 4},
            {"ROL", &C6502::ROL, &C6502::ABS, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"BMI", &C6502::BMI, &C6502::REL, 2},
            {"AND", &C6502::AND, &C6502::IZY, 5},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"AND", &C6502::AND, &C6502::ZPX, 4},
            {"ROL", &C6502::ROL, &C6502::ZPX, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"SEC", &C6502::SEC, &C6502::IMP, 2},
            {"AND", &C6502::AND, &C6502::ABY, 4},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 7},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"AND", &C6502::AND, &C6502::ABX, 4},
            {"ROL", &C6502::ROL, &C6502::ABX, 7},
            {"???", nullptr,     &C6502::IMP, 7},
            {"RTI", &C6502::RTI, &C6502::IMP, 6},
            {"EOR", &C6502::EOR, &C6502::IZX, 6},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 3},
            {"EOR", &C6502::EOR, &C6502::ZP0, 3},
            {"LSR", &C6502::LSR, &C6502::ZP0, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"PHA", &C6502::PHA, &C6502::IMP, 3},
            {"EOR", &C6502::EOR, &C6502::IMM, 2},
            {"LSR", &C6502::LSR, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 2},
            {"JMP", &C6502::JMP, &C6502::ABS, 3},
            {"EOR", &C6502::EOR, &C6502::ABS, 4},
            {"LSR", &C6502::LSR, &C6502::ABS, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"BVC", &C6502::BVC, &C6502::REL, 2},
            {"EOR", &C6502::EOR, &C6502::IZY, 5},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"EOR", &C6502::EOR, &C6502::ZPX, 4},
            {"LSR", &C6502::LSR, &C6502::ZPX, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"CLI", &C6502::CLI, &C6502::IMP, 2},
            {"EOR", &C6502::EOR, &C6502::ABY, 4},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 7},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"EOR", &C6502::EOR, &C6502::ABX, 4},
            {"LSR", &C6502::LSR, &C6502::ABX, 7},
            {"???", nullptr,     &C6502::IMP, 7},
            {"RTS", &C6502::RTS, &C6502::IMP, 6},
            {"ADC", &C6502::ADC, &C6502::IZX, 6},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 3},
            {"ADC", &C6502::ADC, &C6502::ZP0, 3},
            {"ROR", &C6502::ROR, &C6502::ZP0, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"PLA", &C6502::PLA, &C6502::IMP, 4},
            {"ADC", &C6502::ADC, &C6502::IMM, 2},
            {"ROR", &C6502::ROR, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 2},
            {"JMP", &C6502::JMP, &C6502::IND, 5},
            {"ADC", &C6502::ADC, &C6502::ABS, 4},
            {"ROR", &C6502::ROR, &C6502::ABS, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"BVS", &C6502::BVS, &C6502::REL, 2},
            {"ADC", &C6502::ADC, &C6502::IZY, 5},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"ADC", &C6502::ADC, &C6502::ZPX, 4},
            {"ROR", &C6502::ROR, &C6502::ZPX, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"SEI", &C6502::SEI, &C6502::IMP, 2},
            {"ADC", &C6502::ADC, &C6502::ABY, 4},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 7},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"ADC", &C6502::ADC, &C6502::ABX, 4},
            {"ROR", &C6502::ROR, &C6502::ABX, 7},
            {"???", nullptr,     &C6502::IMP, 7},
            {"???", nullptr,     &C6502::IMP, 2},
            {"STA", &C6502::STA, &C6502::IZX, 6},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 6},
            {"STY", &C6502::STY, &C6502::ZP0, 3},
            {"STA", &C6502::STA, &C6502::ZP0, 3},
            {"STX", &C6502::STX, &C6502::ZP0, 3},
            {"???", nullptr,     &C6502::IMP, 3},
            {"DEY", &C6502::DEY, &C6502::IMP, 2},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"TXA", &C6502::TXA, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 2},
            {"STY", &C6502::STY, &C6502::ABS, 4},
            {"STA", &C6502::STA, &C6502::ABS, 4},
            {"STX", &C6502::STX, &C6502::ABS, 4},
            {"???", nullptr,     &C6502::IMP, 4},
            {"BCC", &C6502::BCC, &C6502::REL, 2},
            {"STA", &C6502::STA, &C6502::IZY, 6},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 6},
            {"STY", &C6502::STY, &C6502::ZPX, 4},
            {"STA", &C6502::STA, &C6502::ZPX, 4},
            {"STX", &C6502::STX, &C6502::ZPY, 4},
            {"???", nullptr,     &C6502::IMP, 4},
            {"TYA", &C6502::TYA, &C6502::IMP, 2},
            {"STA", &C6502::STA, &C6502::ABY, 5},
            {"TXS", &C6502::TXS, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 5},
            {"???", &C6502::NOP, &C6502::IMP, 5},
            {"STA", &C6502::STA, &C6502::ABX, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"LDY", &C6502::LDY, &C6502::IMM, 2},
            {"LDA", &C6502::LDA, &C6502::IZX, 6},
            {"LDX", &C6502::LDX, &C6502::IMM, 2},
            {"???", nullptr,     &C6502::IMP, 6},
            {"LDY", &C6502::LDY, &C6502::ZP0, 3},
            {"LDA", &C6502::LDA, &C6502::ZP0, 3},
            {"LDX", &C6502::LDX, &C6502::ZP0, 3},
            {"???", nullptr,     &C6502::IMP, 3},
            {"TAY", &C6502::TAY, &C6502::IMP, 2},
            {"LDA", &C6502::LDA, &C6502::IMM, 2},
            {"TAX", &C6502::TAX, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 2},
            {"LDY", &C6502::LDY, &C6502::ABS, 4},
            {"LDA", &C6502::LDA, &C6502::ABS, 4},
            {"LDX", &C6502::LDX, &C6502::ABS, 4},
            {"???", nullptr,     &C6502::IMP, 4},
            {"BCS", &C6502::BCS, &C6502::REL, 2},
            {"LDA", &C6502::LDA, &C6502::IZY, 5},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 5},
            {"LDY", &C6502::LDY, &C6502::ZPX, 4},
            {"LDA", &C6502::LDA, &C6502::ZPX, 4},
            {"LDX", &C6502::LDX, &C6502::ZPY, 4},
            {"???", nullptr,     &C6502::IMP, 4},
            {"CLV", &C6502::CLV, &C6502::IMP, 2},
            {"LDA", &C6502::LDA, &C6502::ABY, 4},
            {"TSX", &C6502::TSX, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 4},
            {"LDY", &C6502::LDY, &C6502::ABX, 4},
            {"LDA", &C6502::LDA, &C6502::ABX, 4},
            {"LDX", &C6502::LDX, &C6502::ABY, 4},
            {"???", nullptr,     &C6502::IMP, 4},
            {"CPY", &C6502::CPY, &C6502::IMM, 2},
            {"CMP", &C6502::CMP, &C6502::IZX, 6},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"CPY", &C6502::CPY, &C6502::ZP0, 3},
            {"CMP", &C6502::CMP, &C6502::ZP0, 3},
            {"DEC", &C6502::DEC, &C6502::ZP0, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"INY", &C6502::INY, &C6502::IMP, 2},
            {"CMP", &C6502::CMP, &C6502::IMM, 2},
            {"DEX", &C6502::DEX, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 2},
            {"CPY", &C6502::CPY, &C6502::ABS, 4},
            {"CMP", &C6502::CMP, &C6502::ABS, 4},
            {"DEC", &C6502::DEC, &C6502::ABS, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"BNE", &C6502::BNE, &C6502::REL, 2},
            {"CMP", &C6502::CMP, &C6502::IZY, 5},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"CMP", &C6502::CMP, &C6502::ZPX, 4},
            {"DEC", &C6502::DEC, &C6502::ZPX, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"CLD", &C6502::CLD, &C6502::IMP, 2},
            {"CMP", &C6502::CMP, &C6502::ABY, 4},
            {"NOP", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 7},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"CMP", &C6502::CMP, &C6502::ABX, 4},
            {"DEC", &C6502::DEC, &C6502::ABX, 7},
            {"???", nullptr,     &C6502::IMP, 7},
            {"CPX", &C6502::CPX, &C6502::IMM, 2},
            {"SBC", &C6502::SBC, &C6502::IZX, 6},
            {"???", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"CPX", &C6502::CPX, &C6502::ZP0, 3},
            {"SBC", &C6502::SBC, &C6502::ZP0, 3},
            {"INC", &C6502::INC, &C6502::ZP0, 5},
            {"???", nullptr,     &C6502::IMP, 5},
            {"INX", &C6502::INX, &C6502::IMP, 2},
            {"SBC", &C6502::SBC, &C6502::IMM, 2},
            {"NOP", &C6502::NOP, &C6502::IMP, 2},
            {"???", &C6502::SBC, &C6502::IMP, 2},
            {"CPX", &C6502::CPX, &C6502::ABS, 4},
            {"SBC", &C6502::SBC, &C6502::ABS, 4},
            {"INC", &C6502::INC, &C6502::ABS, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"BEQ", &C6502::BEQ, &C6502::REL, 2},
            {"SBC", &C6502::SBC, &C6502::IZY, 5},
            {"???", nullptr,     &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 8},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"SBC", &C6502::SBC, &C6502::ZPX, 4},
            {"INC", &C6502::INC, &C6502::ZPX, 6},
            {"???", nullptr,     &C6502::IMP, 6},
            {"SED", &C6502::SED, &C6502::IMP, 2},
            {"SBC", &C6502::SBC, &C6502::ABY, 4},
            {"NOP", &C6502::NOP, &C6502::IMP, 2},
            {"???", nullptr,     &C6502::IMP, 7},
            {"???", &C6502::NOP, &C6502::IMP, 4},
            {"SBC", &C6502::SBC, &C6502::ABX, 4},
            {"INC", &C6502::INC, &C6502::ABX, 7},
            {"???", nullptr,     &C6502::IMP, 7},
    };
}

void C6502::printf() const {
    std::cout << "[CPU]" << std::endl;
    std::cout << "Reg. A: " << hexof(a) << std::endl;
    std::cout << "Reg. X: " << hexof(x) << std::endl;
    std::cout << "Reg. Y: " << hexof(y) << std::endl;
    std::cout << "Stack ptr: " << hexof(stackp) << std::endl;
    std::cout << "Program counter: " << hexof(pc) << std::endl;
    std::cout << "Status: " << hexof(status) << std::endl;
    std::cout << "OpCode: " << hexof(opcode) << " (" << inst.name << ")" << std::endl;
    std::cout << std::endl;
}

void C6502::step() {

    cycles = 0;
    clock();
    printf();
}

bool C6502::getf(C6502::flag_t flag) const {
    return (status & flag) > 0;
}

void C6502::setf(C6502::flag_t flag, bool value) {
    if (value)
        status |= flag;
    else
        status &= ~flag;
}

void C6502::clock() {
    if (cycles == 0) {
        // parse instruction
        opcode = bus->read(pc++);
        inst = instructions[opcode];
        if (inst.run == nullptr)
            throw cpu_error("unexpected opcode received " + hexof(opcode));

        // set internal cycles counter
        cycles = inst.cycles;

        // execute instruction
        bool add_cycle_1 = (this->*inst.addrmode)();
        bool add_cycle_2 = (this->*inst.run)();

        // if both address mode and instruction agree for an additional cycle,
        // then increment internal cycles count
        if (add_cycle_1 && add_cycle_2)
            cycles++;
    }
    cycles--;
}

void C6502::reset() {
    a = 0x00;
    x = 0x00;
    y = 0x00;
    stackp = 0xFD;
    status = 0x00 | U;

    // read program begin
    addr_abs = PROG_BEGIN_PTR;
    uint16_t lo = bus->read(addr_abs);
    uint16_t hi = bus->read(addr_abs + 1);
    pc = (hi << 8) | lo;

    addr_abs = 0x0000;
    addr_rel = 0x0000;
    opcode = 0x00;
    inst = {"???", nullptr, &C6502::IMP, 0};
    cycles = 8;
}

void C6502::irq() {
    if (!getf(I)) {
        interrupt();
        cycles = 7;
    }
}

void C6502::nmi() {
    interrupt();
    cycles = 8;
}

void C6502::interrupt() {
    push((pc >> 8) & 0x00FF);
    push(pc & 0x00FF);

    setf(B, false);
    setf(U, true);
    setf(I, true);
    push(status);

    addr_abs = INT_ROUTINE_PTR;
    uint16_t lo = bus->read(addr_abs);
    uint16_t hi = bus->read(addr_abs + 1);
    pc = (hi << 8) | lo;
}

// - Addressing modes --------------------------------------------------------------------------------------------------

bool C6502::IMP() {
    return false;
}

bool C6502::IMM() {
    addr_abs = pc++;
    return false;
}

bool C6502::REL() {
    addr_rel = bus->read(pc++);
    if (addr_rel & 0x80)
        addr_rel |= 0xFF00;  // signed byte
    return false;
}

bool C6502::ZP0() {
    addr_abs = bus->read(pc++);
    addr_abs &= 0x00FF;  // page zero
    return false;
}

bool C6502::ZPX() {
    addr_abs = bus->read(pc++) + x;
    addr_abs &= 0x00FF;  // page zero
    return false;
}

bool C6502::ZPY() {
    addr_abs = bus->read(pc++) + y;
    addr_abs &= 0x00FF;  // page zero
    return false;
}

bool C6502::ABS() {
    uint16_t lo = bus->read(pc++);
    uint16_t hi = bus->read(pc++);
    addr_abs = (hi << 8) | lo;
    return false;
}

bool C6502::ABX() {
    uint16_t lo = bus->read(pc++);
    uint16_t hi = bus->read(pc++);
    addr_abs = ((hi << 8) | lo) + x;

    return !is_same_page(addr_abs, hi << 8);
}

bool C6502::ABY() {
    uint16_t lo = bus->read(pc++);
    uint16_t hi = bus->read(pc++);
    addr_abs = ((hi << 8) | lo) + y;

    return !is_same_page(addr_abs, hi << 8);
}

bool C6502::IND() {
    uint16_t lop = bus->read(pc++);
    uint16_t hip = bus->read(pc++);
    uint16_t addr = (hip << 8) | lop;

    uint16_t lo = bus->read(addr);
    uint16_t hi;
    if (lop == 0x00FF)  // Simulate page boundary hardware bug
        hi = bus->read(addr & 0xFF);
    else
        hi = bus->read(addr + 1);

    addr_abs = (hi << 8) | lo;
    return false;
}

bool C6502::IZX() {
    uint16_t addr = ((uint16_t) bus->read(pc++) + (uint16_t) x) & 0x00FF;  // page zero
    uint16_t lo = bus->read(addr);
    uint16_t hi = bus->read((addr + 1) & 0x00FF);

    addr_abs = (hi << 8) | lo;
    return false;
}

bool C6502::IZY() {
    uint16_t addr = bus->read(pc++);

    uint16_t lo = bus->read(addr);
    uint16_t hi = bus->read((addr + 1) & 0x00FF);

    addr_abs = ((hi << 8) | lo) + y;
    return is_same_page(addr_abs, hi << 8);
}

// Instructions - utilities

uint8_t C6502::fetch() {
    if (inst.addrmode == &C6502::IMP)
        throw cpu_error("invoked fetch() with Implied Addressing");
    return bus->read(addr_abs);
}

void C6502::push(uint8_t value) {
    bus->write(STACK_BEGIN + stackp--, value);
}

uint8_t C6502::pop() {
    return bus->read(STACK_BEGIN + ++stackp);
}

void C6502::branch() {
    addr_abs = pc + addr_rel;
    if (is_same_page(addr_abs, pc))
        cycles += 1;
    else
        cycles += 2;
    pc = addr_abs;
}

// Instructions

bool C6502::ADC() {
    uint16_t fetched = fetch();
    uint16_t sum = ((uint16_t) a) + fetched + (getf(C) ? 1 : 0) + (getf(D) ? 1 : 0);

    setf(C, sum > 0x00FF);
    setf(Z, (sum & 0x00FF) == 0);
    setf(N, (sum & 0x8000) > 0);
    setf(V, (~((uint16_t) a ^ fetched) & ((uint16_t) a ^ sum)) & 0x0080);

    a = sum & 0x00FF;

    return true;
}

bool C6502::AND() {
    a = a & fetch();
    setf(Z, a == 0x00);
    setf(N, a & 0x80);
    return true;
}

// TODO: check differs from OLC implementation
bool C6502::ASL() {
    bool is_reg_a = (inst.addrmode == &C6502::IMP);  // Implied means register A

    uint16_t value = is_reg_a ? a : fetch();
    value <<= 1;

    setf(C, (value & 0xFF00) > 0);
    value &= 0x00FF;
    setf(N, (value & 0x0080) > 0);
    setf(Z, value == 0x0000);

    if (is_reg_a)
        a = value;
    else
        bus->write(addr_abs, value);

    return !is_reg_a;
}

bool C6502::BCC() {
    if (!getf(C))
        branch();

    return false;
}

bool C6502::BCS() {
    if (getf(C))
        branch();

    return false;
}

bool C6502::BEQ() {
    if (getf(Z))
        branch();

    return false;
}

bool C6502::BIT() {
    uint8_t fetched = fetch();
    setf(N, (fetched & N) > 0);
    setf(V, (fetched & V) > 0);
    setf(Z, (a & fetched) == 0x00);

    return false;
}

bool C6502::BMI() {
    if (getf(N))
        branch();

    return false;
}

bool C6502::BNE() {
    if (!getf(Z))
        branch();

    return false;
}

bool C6502::BPL() {
    if (!getf(N))
        branch();

    return false;
}

bool C6502::BRK() {
    pc++;

    setf(I, true);
    push(pc >> 8);
    push(pc);

    setf(B, true);
    push(status);
    setf(B, false);

    addr_abs = INT_ROUTINE_PTR;
    uint16_t lo = bus->read(addr_abs);
    uint16_t hi = bus->read(addr_abs + 1);
    pc = (hi << 8) | lo;

    return false;
}

bool C6502::BVC() {
    if (!getf(V))
        branch();

    return false;
}

bool C6502::BVS() {
    if (getf(V))
        branch();

    return false;
}

bool C6502::CLC() {
    setf(C, false);
    return false;
}

bool C6502::CLD() {
    setf(D, false);
    return false;
}

bool C6502::CLI() {
    setf(I, false);
    return false;
}

bool C6502::CLV() {
    setf(V, false);
    return false;
}

bool C6502::CMP() {
    uint16_t diff = ((uint16_t) a) - fetch();
    setf(C, diff > 0x00FF);
    setf(Z, (diff & 0x00FF) == 0x0000);
    setf(N, (diff & 0x0080) > 0);
    return true;
}

bool C6502::CPX() {
    uint16_t diff = ((uint16_t) x) - fetch();
    setf(C, diff > 0x00FF);
    setf(Z, (diff & 0x00FF) == 0x0000);
    setf(N, (diff & 0x0080) > 0);
    return true;
}

bool C6502::CPY() {
    uint16_t diff = ((uint16_t) y) - fetch();
    setf(C, diff > 0x00FF);
    setf(Z, (diff & 0x00FF) == 0x0000);
    setf(N, (diff & 0x0080) > 0);
    return true;
}

bool C6502::DEC() {
    uint8_t value = fetch();
    value--;
    bus->write(addr_abs, value);

    setf(Z, value == 0x00);
    setf(N, (value & 0x80) > 0);
    return true;
}

bool C6502::DEX() {
    x--;
    setf(Z, x == 0x00);
    setf(N, (x & 0x80) > 0);
    return false;
}

bool C6502::DEY() {
    y--;
    setf(Z, y == 0x00);
    setf(N, (y & 0x80) > 0);
    return false;
}

bool C6502::EOR() {
    a = a ^ fetch();
    setf(Z, a == 0x00);
    setf(N, a & 0x80);
    return true;
}

bool C6502::INC() {
    uint8_t value = fetch();
    value++;
    bus->write(addr_abs, value);

    setf(Z, value == 0x00);
    setf(N, (value & 0x80) > 0);
    return true;
}

bool C6502::INX() {
    x++;
    setf(Z, x == 0x00);
    setf(N, x & 0x80);
    return false;
}

bool C6502::INY() {
    y++;
    setf(Z, y == 0x00);
    setf(N, y & 0x80);
    return false;
}

bool C6502::JMP() {
    pc = addr_abs;
    return false;
}

bool C6502::JSR() {
    pc--;
    push(pc >> 8);
    push(pc);

    pc = addr_abs;
    return false;
}

bool C6502::LDA() {
    a = fetch();
    setf(Z, a == 0x00);
    setf(N, a & 0x80);
    return true;
}

bool C6502::LDX() {
    x = fetch();
    setf(Z, x == 0x00);
    setf(N, x & 0x80);
    return true;
}

bool C6502::LDY() {
    y = fetch();
    setf(Z, y == 0x00);
    setf(N, y & 0x80);
    return true;
}

bool C6502::LSR() {
    bool is_reg_a = (inst.addrmode == &C6502::IMP);  // Implied means register A

    uint8_t value = is_reg_a ? a : fetch();
    setf(C, (value & 0x0001) > 0);
    value >>= 1;
    setf(N, false);
    setf(Z, value == 0x00);

    if (is_reg_a)
        a = value;
    else
        bus->write(addr_abs, value);

    return !is_reg_a;
}

bool C6502::NOP() {
    // Not all NOPs are equal!
    // https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
    switch (opcode) {
        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC:
            return true;
    }
    return false;
}

bool C6502::ORA() {
    a = a | fetch();
    setf(Z, a == 0x00);
    setf(N, a & 0x80);
    return true;
}

bool C6502::PHA() {
    push(a);
    return false;
}

bool C6502::PHP() {
    push(status);
    return false;
}

bool C6502::PLA() {
    a = pop();
    setf(N, (a & 0x80) > 0);
    setf(Z, a == 0x00);
    return false;
}

bool C6502::PLP() {
    status = pop() | U | B;
    return false;
}

bool C6502::ROL() {
    bool is_reg_a = (inst.addrmode == &C6502::IMP);  // Implied means register A

    uint16_t value = is_reg_a ? a : fetch();
    value = (value << 1) | (getf(C) ? 1 : 0);
    setf(C, (value & 0x0100) > 0);
    value &= 0x00FF;
    setf(N, (value & 0x0080) > 0);
    setf(Z, value == 0x0000);

    if (is_reg_a)
        a = value;
    else
        bus->write(addr_abs, value);

    return !is_reg_a;
}

bool C6502::ROR() {
    bool is_reg_a = (inst.addrmode == &C6502::IMP);  // Implied means register A

    uint8_t value = is_reg_a ? a : fetch();
    uint8_t new_c = value & 0x01;
    value = (value >> 1) | (getf(C) ? 0x80 : 0);
    setf(C, new_c > 0);
    value &= 0x00FF;
    setf(N, (value & 0x0080) > 0);
    setf(Z, value == 0x0000);

    if (is_reg_a)
        a = value;
    else
        bus->write(addr_abs, value);

    return !is_reg_a;
}

bool C6502::RTI() {
    status = pop() | B | U;
    uint16_t lo = pop();
    uint16_t hi = pop();
    pc = (hi << 8) | lo;
    return false;
}

bool C6502::RTS() {
    uint16_t lo = pop();
    uint16_t hi = pop();
    pc = (hi << 8) | lo;
    pc++;
    return false;
}

bool C6502::SBC() {
    uint16_t fetched = ((uint16_t) fetch()) ^ 0x00FF;

    uint16_t diff = ((uint16_t) a) + fetched + (getf(C) ? 1 : 0) + (getf(D) ? 1 : 0);

    setf(C, diff > 0x00FF);
    setf(Z, (diff & 0x00FF) == 0);
    setf(N, (diff & 0x8000) > 0);
    setf(V, (diff ^ (uint16_t) a) & (diff ^ fetched) & 0x0080);

    a = diff & 0x00FF;

    return true;
}

bool C6502::SEC() {
    setf(C, true);
    return false;
}

bool C6502::SED() {
    setf(D, true);
    return false;
}

bool C6502::SEI() {
    setf(I, true);
    return false;
}

bool C6502::STA() {
    bus->write(addr_abs, a);
    return false;
}

bool C6502::STX() {
    bus->write(addr_abs, x);
    return false;
}

bool C6502::STY() {
    bus->write(addr_abs, y);
    return false;
}

bool C6502::TAX() {
    x = a;
    setf(N, (a & 0x80) > 0);
    setf(Z, a == 0x00);
    return false;
}

bool C6502::TAY() {
    y = a;
    setf(N, (a & 0x80) > 0);
    setf(Z, a == 0x00);
    return false;
}

bool C6502::TSX() {
    x = status;
    setf(N, (status & 0x80) > 0);
    setf(Z, status == 0x00);
    return false;
}

bool C6502::TXA() {
    a = x;
    setf(N, (x & 0x80) > 0);
    setf(Z, x == 0x00);
    return false;
}

bool C6502::TXS() {
    status = x;
    return false;
}

bool C6502::TYA() {
    a = y;
    setf(N, (y & 0x80) > 0);
    setf(Z, y == 0x00);
    return false;
}




