//
// Created by Davide Caroselli on 04/03/23.
//

#include "C6502.h"

using namespace std;

void C6502::clock() {
    if (cycles_ == 0) {
        // parse instruction
        opcode = read_pc();
        instruction = const_cast<inst_t *>(&INSTRUCTIONS[opcode]);
        if (instruction->name == "???")
            Logger::warn("CPU: unexpected opcode %2X", opcode);

        // set internal cycles_ counter
        cycles_ = instruction->cycles;

        // execute instruction
        bool add_cycle1 = (this->*instruction->addrmode.f)();
        bool add_cycle2 = (this->*instruction->op)();

        // if both address mode and instruction agree for an additional cycle,
        // then increment internal cycles_ count
        if (add_cycle1 && add_cycle2) cycles_++;
    }

    cycles_ -= 1;
}

void C6502::reset() {
    a = 0x00;
    x = 0x00;
    y = 0x00;
    stack_ptr = 0xFD;
    status = 0x00 | U;

    // read program begin
    abs_addr = PROG_BEGIN_ADDR;
    uint16_t lo = bus.read(abs_addr);
    uint16_t hi = bus.read(abs_addr + 1);
    prg_counter = (hi << 8) | lo;

    abs_addr = 0x0000;
    rel_addr = 0x0000;
    opcode = 0x00;
    instruction = nullptr;
    cycles_ = 8;
}

void C6502::irq() {
    if (!get_status(I)) {
        interrupt();
        cycles_ = 7;
    }
}

void C6502::nmi() {
    interrupt();
    cycles_ = 8;
}

void C6502::interrupt() {
    push((prg_counter >> 8) & 0x00FF);
    push(prg_counter & 0x00FF);

    set_status(B, false);
    set_status(U, true);
    set_status(I, true);
    push(status);

    abs_addr = INT_ROUTINE_ADDR;
    uint16_t lo = bus.read(abs_addr);
    uint16_t hi = bus.read(abs_addr + 1);
    prg_counter = (hi << 8) | lo;
}

void C6502::branch() {
    abs_addr = prg_counter + rel_addr;
    cycles_++;
    if (!is_same_page(abs_addr, prg_counter)) cycles_++;
    prg_counter = abs_addr;
}

// - Addressing modes --------------------------------------------------------------------------------------------------

addrmode_t IMP("IMP", 0, &C6502::IMP_);

bool C6502::IMP_() {
    return false;
}

addrmode_t IMM("IMM", 1, &C6502::IMM_);

bool C6502::IMM_() {
    abs_addr = prg_counter++;
    return false;
}

addrmode_t REL("REL", 1, &C6502::REL_);

bool C6502::REL_() {
    rel_addr = read_pc();
    if (rel_addr & 0x80)
        rel_addr |= 0xFF00;  // signed byte
    return false;
}

addrmode_t ZP0("ZP0", 1, &C6502::ZP0_);

bool C6502::ZP0_() {
    abs_addr = read_pc();
    abs_addr &= 0x00FF;  // page zero
    return false;
}

addrmode_t ZPX("ZPX", 1, &C6502::ZPX_);

bool C6502::ZPX_() {
    abs_addr = read_pc() + x;
    abs_addr &= 0x00FF;  // page zero
    return false;
}

addrmode_t ZPY("ZPY", 1, &C6502::ZPY_);

bool C6502::ZPY_() {
    abs_addr = read_pc() + y;
    abs_addr &= 0x00FF;  // page zero
    return false;
}

addrmode_t ABS("ABS", 2, &C6502::ABS_);

bool C6502::ABS_() {
    uint16_t lo = read_pc();
    uint16_t hi = read_pc();
    abs_addr = (hi << 8) | lo;
    return false;
}

addrmode_t ABX("ABX", 2, &C6502::ABX_);

bool C6502::ABX_() {
    uint16_t lo = read_pc();
    uint16_t hi = read_pc();
    abs_addr = ((hi << 8) | lo) + x;

    return !is_same_page(abs_addr, hi << 8);
}

addrmode_t ABY("ABY", 2, &C6502::ABY_);

bool C6502::ABY_() {
    uint16_t lo = read_pc();
    uint16_t hi = read_pc();
    abs_addr = ((hi << 8) | lo) + y;

    return !is_same_page(abs_addr, hi << 8);
}

addrmode_t IND("IND", 2, &C6502::IND_);

bool C6502::IND_() {
    uint16_t lop = read_pc();
    uint16_t hip = read_pc();
    uint16_t addr = (hip << 8) | lop;

    uint16_t lo = bus.read(addr);
    uint16_t hi;
    if (lop == 0x00FF)  // Simulate page boundary hardware bug
        hi = bus.read(addr & 0xFF);
    else
        hi = bus.read(addr + 1);

    abs_addr = (hi << 8) | lo;
    return false;
}

addrmode_t IZX("IZX", 1, &C6502::IZX_);

bool C6502::IZX_() {
    uint16_t addr = ((uint16_t) read_pc() + (uint16_t) x) & 0x00FF;  // page zero
    uint16_t lo = bus.read(addr);
    uint16_t hi = bus.read((addr + 1) & 0x00FF);

    abs_addr = (hi << 8) | lo;
    return false;
}

addrmode_t IZY("IZY", 1, &C6502::IZY_);

bool C6502::IZY_() {
    uint16_t addr = read_pc();

    uint16_t lo = bus.read(addr);
    uint16_t hi = bus.read((addr + 1) & 0x00FF);

    abs_addr = ((hi << 8) | lo) + y;
    return is_same_page(abs_addr, hi << 8);
}

// - Instructions ------------------------------------------------------------------------------------------------------

bool C6502::ADC() {
    uint16_t fetched = fetch();
    uint16_t sum = ((uint16_t) a) + fetched + (get_status(C) ? 1 : 0) + (get_status(D) ? 1 : 0);

    set_status(C, sum > 0x00FF);
    set_status(Z, (sum & 0x00FF) == 0);
    set_status(N, (sum & 0x8000) > 0);
    set_status(V, (~((uint16_t) a ^ fetched) & ((uint16_t) a ^ sum)) & 0x0080);

    a = sum & 0x00FF;

    return true;
}

bool C6502::AND() {
    a = a & fetch();
    set_status(Z, a == 0x00);
    set_status(N, a & 0x80);
    return true;
}

bool C6502::ASL() {
    bool is_reg_a = (instruction->addrmode.name == "IMP");  // Implied means register A

    uint16_t value = is_reg_a ? a : fetch();
    value <<= 1;

    set_status(C, (value & 0xFF00) > 0);
    value &= 0x00FF;
    set_status(N, (value & 0x0080) > 0);
    set_status(Z, value == 0x0000);

    if (is_reg_a)
        a = value;
    else
        bus.write(abs_addr, value);

    return !is_reg_a;
}

bool C6502::BCC() {
    if (!get_status(C))
        branch();

    return false;
}

bool C6502::BCS() {
    if (get_status(C))
        branch();

    return false;
}

bool C6502::BEQ() {
    if (get_status(Z))
        branch();

    return false;
}

bool C6502::BIT() {
    uint8_t fetched = fetch();
    set_status(N, (fetched & N) > 0);
    set_status(V, (fetched & V) > 0);
    set_status(Z, (a & fetched) == 0x00);

    return false;
}

bool C6502::BMI() {
    if (get_status(N))
        branch();

    return false;
}

bool C6502::BNE() {
    if (!get_status(Z))
        branch();

    return false;
}

bool C6502::BPL() {
    if (!get_status(N))
        branch();

    return false;
}

bool C6502::BRK() {
    prg_counter++;

    set_status(I, true);
    push(prg_counter >> 8);
    push(prg_counter);

    set_status(B, true);
    push(status);
    set_status(B, false);

    abs_addr = INT_ROUTINE_ADDR;
    uint16_t lo = bus.read(abs_addr);
    uint16_t hi = bus.read(abs_addr + 1);
    prg_counter = (hi << 8) | lo;

    return false;
}

bool C6502::BVC() {
    if (!get_status(V))
        branch();

    return false;
}

bool C6502::BVS() {
    if (get_status(V))
        branch();

    return false;
}

bool C6502::CLC() {
    set_status(C, false);
    return false;
}

bool C6502::CLD() {
    set_status(D, false);
    return false;
}

bool C6502::CLI() {
    set_status(I, false);
    return false;
}

bool C6502::CLV() {
    set_status(V, false);
    return false;
}

bool C6502::CMP() {
    uint16_t diff = ((uint16_t) a) - fetch();
    set_status(C, diff > 0x00FF);
    set_status(Z, (diff & 0x00FF) == 0x0000);
    set_status(N, (diff & 0x0080) > 0);
    return true;
}

bool C6502::CPX() {
    uint16_t diff = ((uint16_t) x) - fetch();
    set_status(C, diff > 0x00FF);
    set_status(Z, (diff & 0x00FF) == 0x0000);
    set_status(N, (diff & 0x0080) > 0);
    return true;
}

bool C6502::CPY() {
    uint16_t diff = ((uint16_t) y) - fetch();
    set_status(C, diff > 0x00FF);
    set_status(Z, (diff & 0x00FF) == 0x0000);
    set_status(N, (diff & 0x0080) > 0);
    return true;
}

bool C6502::DEC() {
    uint8_t value = fetch();
    value--;
    bus.write(abs_addr, value);

    set_status(Z, value == 0x00);
    set_status(N, (value & 0x80) > 0);
    return true;
}

bool C6502::DEX() {
    x--;
    set_status(Z, x == 0x00);
    set_status(N, (x & 0x80) > 0);
    return false;
}

bool C6502::DEY() {
    y--;
    set_status(Z, y == 0x00);
    set_status(N, (y & 0x80) > 0);
    return false;
}

bool C6502::EOR() {
    a = a ^ fetch();
    set_status(Z, a == 0x00);
    set_status(N, a & 0x80);
    return true;
}

bool C6502::INC() {
    uint8_t value = fetch();
    value++;
    bus.write(abs_addr, value);

    set_status(Z, value == 0x00);
    set_status(N, (value & 0x80) > 0);
    return true;
}

bool C6502::INX() {
    x++;
    set_status(Z, x == 0x00);
    set_status(N, x & 0x80);
    return false;
}

bool C6502::INY() {
    y++;
    set_status(Z, y == 0x00);
    set_status(N, y & 0x80);
    return false;
}

bool C6502::JMP() {
    prg_counter = abs_addr;
    return false;
}

bool C6502::JSR() {
    prg_counter--;
    push(prg_counter >> 8);
    push(prg_counter);

    prg_counter = abs_addr;
    return false;
}

bool C6502::LDA() {
    a = fetch();
    set_status(Z, a == 0x00);
    set_status(N, a & 0x80);
    return true;
}

bool C6502::LDX() {
    x = fetch();
    set_status(Z, x == 0x00);
    set_status(N, x & 0x80);
    return true;
}

bool C6502::LDY() {
    y = fetch();
    set_status(Z, y == 0x00);
    set_status(N, y & 0x80);
    return true;
}

bool C6502::LSR() {
    bool is_reg_a = (instruction->addrmode.name == "IMP");  // Implied means register A

    uint8_t value = is_reg_a ? a : fetch();
    set_status(C, (value & 0x0001) > 0);
    value >>= 1;
    set_status(N, false);
    set_status(Z, value == 0x00);

    if (is_reg_a)
        a = value;
    else
        bus.write(abs_addr, value);

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
    set_status(Z, a == 0x00);
    set_status(N, a & 0x80);
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
    set_status(N, (a & 0x80) > 0);
    set_status(Z, a == 0x00);
    return false;
}

bool C6502::PLP() {
    status = pop() | U | B;
    return false;
}

bool C6502::ROL() {
    bool is_reg_a = (instruction->addrmode.name == "IMP");  // Implied means register A

    uint16_t value = is_reg_a ? a : fetch();
    value = (value << 1) | (get_status(C) ? 1 : 0);
    set_status(C, (value & 0x0100) > 0);
    value &= 0x00FF;
    set_status(N, (value & 0x0080) > 0);
    set_status(Z, value == 0x0000);

    if (is_reg_a)
        a = value;
    else
        bus.write(abs_addr, value);

    return !is_reg_a;
}

bool C6502::ROR() {
    bool is_reg_a = (instruction->addrmode.name == "IMP");  // Implied means register A

    uint8_t value = is_reg_a ? a : fetch();
    uint8_t new_c = value & 0x01;
    value = (value >> 1) | (get_status(C) ? 0x80 : 0);
    set_status(C, new_c > 0);
    value &= 0x00FF;
    set_status(N, (value & 0x0080) > 0);
    set_status(Z, value == 0x0000);

    if (is_reg_a)
        a = value;
    else
        bus.write(abs_addr, value);

    return !is_reg_a;
}

bool C6502::RTI() {
    status = pop() | B | U;
    uint16_t lo = pop();
    uint16_t hi = pop();
    prg_counter = (hi << 8) | lo;
    return false;
}

bool C6502::RTS() {
    uint16_t lo = pop();
    uint16_t hi = pop();
    prg_counter = (hi << 8) | lo;
    prg_counter++;
    return false;
}

bool C6502::SBC() {
    uint16_t fetched = ((uint16_t) fetch()) ^ 0x00FF;

    uint16_t diff = ((uint16_t) a) + fetched + (get_status(C) ? 1 : 0) + (get_status(D) ? 1 : 0);

    set_status(C, diff > 0x00FF);
    set_status(Z, (diff & 0x00FF) == 0);
    set_status(N, (diff & 0x8000) > 0);
    set_status(V, (diff ^ (uint16_t) a) & (diff ^ fetched) & 0x0080);

    a = diff & 0x00FF;

    return true;
}

bool C6502::SEC() {
    set_status(C, true);
    return false;
}

bool C6502::SED() {
    set_status(D, true);
    return false;
}

bool C6502::SEI() {
    set_status(I, true);
    return false;
}

bool C6502::STA() {
    bus.write(abs_addr, a);
    return false;
}

bool C6502::STX() {
    bus.write(abs_addr, x);
    return false;
}

bool C6502::STY() {
    bus.write(abs_addr, y);
    return false;
}

bool C6502::TAX() {
    x = a;
    set_status(N, (a & 0x80) > 0);
    set_status(Z, a == 0x00);
    return false;
}

bool C6502::TAY() {
    y = a;
    set_status(N, (a & 0x80) > 0);
    set_status(Z, a == 0x00);
    return false;
}

bool C6502::TSX() {
    x = pop();
    set_status(N, (status & 0x80) > 0);
    set_status(Z, status == 0x00);
    return false;
}

bool C6502::TXA() {
    a = x;
    set_status(N, (x & 0x80) > 0);
    set_status(Z, x == 0x00);
    return false;
}

bool C6502::TXS() {
    push(x);
    return false;
}

bool C6502::TYA() {
    a = y;
    set_status(N, (y & 0x80) > 0);
    set_status(Z, y == 0x00);
    return false;
}

bool C6502::UNK() {
    return false;
}

const std::vector<inst_t> C6502::INSTRUCTIONS = {
        {"BRK", &C6502::BRK, IMM, 7},
        {"ORA", &C6502::ORA, IZX, 6},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 3},
        {"ORA", &C6502::ORA, ZP0, 3},
        {"ASL", &C6502::ASL, ZP0, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"PHP", &C6502::PHP, IMP, 3},
        {"ORA", &C6502::ORA, IMM, 2},
        {"ASL", &C6502::ASL, IMP, 2},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::NOP, IMP, 4},
        {"ORA", &C6502::ORA, ABS, 4},
        {"ASL", &C6502::ASL, ABS, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"BPL", &C6502::BPL, REL, 2},
        {"ORA", &C6502::ORA, IZY, 5},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 4},
        {"ORA", &C6502::ORA, ZPX, 4},
        {"ASL", &C6502::ASL, ZPX, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"CLC", &C6502::CLC, IMP, 2},
        {"ORA", &C6502::ORA, ABY, 4},
        {"???", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 7},
        {"???", &C6502::NOP, IMP, 4},
        {"ORA", &C6502::ORA, ABX, 4},
        {"ASL", &C6502::ASL, ABX, 7},
        {"???", &C6502::UNK, IMP, 7},
        {"JSR", &C6502::JSR, ABS, 6},
        {"AND", &C6502::AND, IZX, 6},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"BIT", &C6502::BIT, ZP0, 3},
        {"AND", &C6502::AND, ZP0, 3},
        {"ROL", &C6502::ROL, ZP0, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"PLP", &C6502::PLP, IMP, 4},
        {"AND", &C6502::AND, IMM, 2},
        {"ROL", &C6502::ROL, IMP, 2},
        {"???", &C6502::UNK, IMP, 2},
        {"BIT", &C6502::BIT, ABS, 4},
        {"AND", &C6502::AND, ABS, 4},
        {"ROL", &C6502::ROL, ABS, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"BMI", &C6502::BMI, REL, 2},
        {"AND", &C6502::AND, IZY, 5},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 4},
        {"AND", &C6502::AND, ZPX, 4},
        {"ROL", &C6502::ROL, ZPX, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"SEC", &C6502::SEC, IMP, 2},
        {"AND", &C6502::AND, ABY, 4},
        {"???", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 7},
        {"???", &C6502::NOP, IMP, 4},
        {"AND", &C6502::AND, ABX, 4},
        {"ROL", &C6502::ROL, ABX, 7},
        {"???", &C6502::UNK, IMP, 7},
        {"RTI", &C6502::RTI, IMP, 6},
        {"EOR", &C6502::EOR, IZX, 6},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 3},
        {"EOR", &C6502::EOR, ZP0, 3},
        {"LSR", &C6502::LSR, ZP0, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"PHA", &C6502::PHA, IMP, 3},
        {"EOR", &C6502::EOR, IMM, 2},
        {"LSR", &C6502::LSR, IMP, 2},
        {"???", &C6502::UNK, IMP, 2},
        {"JMP", &C6502::JMP, ABS, 3},
        {"EOR", &C6502::EOR, ABS, 4},
        {"LSR", &C6502::LSR, ABS, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"BVC", &C6502::BVC, REL, 2},
        {"EOR", &C6502::EOR, IZY, 5},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 4},
        {"EOR", &C6502::EOR, ZPX, 4},
        {"LSR", &C6502::LSR, ZPX, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"CLI", &C6502::CLI, IMP, 2},
        {"EOR", &C6502::EOR, ABY, 4},
        {"???", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 7},
        {"???", &C6502::NOP, IMP, 4},
        {"EOR", &C6502::EOR, ABX, 4},
        {"LSR", &C6502::LSR, ABX, 7},
        {"???", &C6502::UNK, IMP, 7},
        {"RTS", &C6502::RTS, IMP, 6},
        {"ADC", &C6502::ADC, IZX, 6},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 3},
        {"ADC", &C6502::ADC, ZP0, 3},
        {"ROR", &C6502::ROR, ZP0, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"PLA", &C6502::PLA, IMP, 4},
        {"ADC", &C6502::ADC, IMM, 2},
        {"ROR", &C6502::ROR, IMP, 2},
        {"???", &C6502::UNK, IMP, 2},
        {"JMP", &C6502::JMP, IND, 5},
        {"ADC", &C6502::ADC, ABS, 4},
        {"ROR", &C6502::ROR, ABS, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"BVS", &C6502::BVS, REL, 2},
        {"ADC", &C6502::ADC, IZY, 5},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 4},
        {"ADC", &C6502::ADC, ZPX, 4},
        {"ROR", &C6502::ROR, ZPX, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"SEI", &C6502::SEI, IMP, 2},
        {"ADC", &C6502::ADC, ABY, 4},
        {"???", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 7},
        {"???", &C6502::NOP, IMP, 4},
        {"ADC", &C6502::ADC, ABX, 4},
        {"ROR", &C6502::ROR, ABX, 7},
        {"???", &C6502::UNK, IMP, 7},
        {"???", &C6502::UNK, IMP, 2},
        {"STA", &C6502::STA, IZX, 6},
        {"???", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 6},
        {"STY", &C6502::STY, ZP0, 3},
        {"STA", &C6502::STA, ZP0, 3},
        {"STX", &C6502::STX, ZP0, 3},
        {"???", &C6502::UNK, IMP, 3},
        {"DEY", &C6502::DEY, IMP, 2},
        {"???", &C6502::NOP, IMP, 2},
        {"TXA", &C6502::TXA, IMP, 2},
        {"???", &C6502::UNK, IMP, 2},
        {"STY", &C6502::STY, ABS, 4},
        {"STA", &C6502::STA, ABS, 4},
        {"STX", &C6502::STX, ABS, 4},
        {"???", &C6502::UNK, IMP, 4},
        {"BCC", &C6502::BCC, REL, 2},
        {"STA", &C6502::STA, IZY, 6},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 6},
        {"STY", &C6502::STY, ZPX, 4},
        {"STA", &C6502::STA, ZPX, 4},
        {"STX", &C6502::STX, ZPY, 4},
        {"???", &C6502::UNK, IMP, 4},
        {"TYA", &C6502::TYA, IMP, 2},
        {"STA", &C6502::STA, ABY, 5},
        {"TXS", &C6502::TXS, IMP, 2},
        {"???", &C6502::UNK, IMP, 5},
        {"???", &C6502::NOP, IMP, 5},
        {"STA", &C6502::STA, ABX, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"LDY", &C6502::LDY, IMM, 2},
        {"LDA", &C6502::LDA, IZX, 6},
        {"LDX", &C6502::LDX, IMM, 2},
        {"???", &C6502::UNK, IMP, 6},
        {"LDY", &C6502::LDY, ZP0, 3},
        {"LDA", &C6502::LDA, ZP0, 3},
        {"LDX", &C6502::LDX, ZP0, 3},
        {"???", &C6502::UNK, IMP, 3},
        {"TAY", &C6502::TAY, IMP, 2},
        {"LDA", &C6502::LDA, IMM, 2},
        {"TAX", &C6502::TAX, IMP, 2},
        {"???", &C6502::UNK, IMP, 2},
        {"LDY", &C6502::LDY, ABS, 4},
        {"LDA", &C6502::LDA, ABS, 4},
        {"LDX", &C6502::LDX, ABS, 4},
        {"???", &C6502::UNK, IMP, 4},
        {"BCS", &C6502::BCS, REL, 2},
        {"LDA", &C6502::LDA, IZY, 5},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 5},
        {"LDY", &C6502::LDY, ZPX, 4},
        {"LDA", &C6502::LDA, ZPX, 4},
        {"LDX", &C6502::LDX, ZPY, 4},
        {"???", &C6502::UNK, IMP, 4},
        {"CLV", &C6502::CLV, IMP, 2},
        {"LDA", &C6502::LDA, ABY, 4},
        {"TSX", &C6502::TSX, IMP, 2},
        {"???", &C6502::UNK, IMP, 4},
        {"LDY", &C6502::LDY, ABX, 4},
        {"LDA", &C6502::LDA, ABX, 4},
        {"LDX", &C6502::LDX, ABY, 4},
        {"???", &C6502::UNK, IMP, 4},
        {"CPY", &C6502::CPY, IMM, 2},
        {"CMP", &C6502::CMP, IZX, 6},
        {"???", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"CPY", &C6502::CPY, ZP0, 3},
        {"CMP", &C6502::CMP, ZP0, 3},
        {"DEC", &C6502::DEC, ZP0, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"INY", &C6502::INY, IMP, 2},
        {"CMP", &C6502::CMP, IMM, 2},
        {"DEX", &C6502::DEX, IMP, 2},
        {"???", &C6502::UNK, IMP, 2},
        {"CPY", &C6502::CPY, ABS, 4},
        {"CMP", &C6502::CMP, ABS, 4},
        {"DEC", &C6502::DEC, ABS, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"BNE", &C6502::BNE, REL, 2},
        {"CMP", &C6502::CMP, IZY, 5},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 4},
        {"CMP", &C6502::CMP, ZPX, 4},
        {"DEC", &C6502::DEC, ZPX, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"CLD", &C6502::CLD, IMP, 2},
        {"CMP", &C6502::CMP, ABY, 4},
        {"NOP", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 7},
        {"???", &C6502::NOP, IMP, 4},
        {"CMP", &C6502::CMP, ABX, 4},
        {"DEC", &C6502::DEC, ABX, 7},
        {"???", &C6502::UNK, IMP, 7},
        {"CPX", &C6502::CPX, IMM, 2},
        {"SBC", &C6502::SBC, IZX, 6},
        {"???", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"CPX", &C6502::CPX, ZP0, 3},
        {"SBC", &C6502::SBC, ZP0, 3},
        {"INC", &C6502::INC, ZP0, 5},
        {"???", &C6502::UNK, IMP, 5},
        {"INX", &C6502::INX, IMP, 2},
        {"SBC", &C6502::SBC, IMM, 2},
        {"NOP", &C6502::NOP, IMP, 2},
        {"???", &C6502::SBC, IMP, 2},
        {"CPX", &C6502::CPX, ABS, 4},
        {"SBC", &C6502::SBC, ABS, 4},
        {"INC", &C6502::INC, ABS, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"BEQ", &C6502::BEQ, REL, 2},
        {"SBC", &C6502::SBC, IZY, 5},
        {"???", &C6502::UNK, IMP, 2},
        {"???", &C6502::UNK, IMP, 8},
        {"???", &C6502::NOP, IMP, 4},
        {"SBC", &C6502::SBC, ZPX, 4},
        {"INC", &C6502::INC, ZPX, 6},
        {"???", &C6502::UNK, IMP, 6},
        {"SED", &C6502::SED, IMP, 2},
        {"SBC", &C6502::SBC, ABY, 4},
        {"NOP", &C6502::NOP, IMP, 2},
        {"???", &C6502::UNK, IMP, 7},
        {"???", &C6502::NOP, IMP, 4},
        {"SBC", &C6502::SBC, ABX, 4},
        {"INC", &C6502::INC, ABX, 7},
        {"???", &C6502::UNK, IMP, 7},
};