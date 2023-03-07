//
// Created by Davide Caroselli on 04/03/23.
//

#include "C6502.h"

using namespace std;

C6502::C6502() : bus(SystemBus::MAIN_BUS_ID), ram(2048, 0x0000, 0x1FFF) {
    bus.connect(&ram);
}

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
        interrupt(IRQ_ROUTINE_ADDR);
        cycles_ = 7;
    }
}

void C6502::nmi() {
    interrupt(0xFFFA);
    cycles_ = 8;
}

void C6502::interrupt(uint16_t location) {
    push((prg_counter >> 8) & 0x00FF);
    push(prg_counter & 0x00FF);

    set_status(B, false);
    set_status(U, true);
    set_status(I, true);
    push(status);

    abs_addr = location;
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

// Add Memory to Accumulator with Carry
// A + M + C -> A, C
bool C6502::ADC() {
    uint16_t fetched = fetch();
    uint16_t sum = ((uint16_t) a) + fetched + (get_status(C) ? 1 : 0);

    set_status(C, sum > 0x00FF);
    set_status(Z, (sum & 0x00FF) == 0);
    set_status(N, (sum & 0x8000) > 0);
    set_status(V, (~((uint16_t) a ^ fetched) & ((uint16_t) a ^ sum)) & 0x0080);

    a = sum & 0x00FF;

    return true;
}

// AND Memory with Accumulator
// A AND M -> A
bool C6502::AND() {
    a = a & fetch();
    set_status(Z, a == 0x00);
    set_status(N, (a & 0x80) > 0);
    return true;
}

// Shift Left One Bit (Memory or Accumulator)
// C <- [76543210] <- 0
bool C6502::ASL() {
    bool is_reg_a = (instruction->addrmode.name == "IMP");  // Implied means register A

    uint16_t value = is_reg_a ? a : fetch();
    value <<= 1;

    set_status(C, (value & 0xFF00) > 0);
    set_status(N, (value & 0x0080) > 0);
    set_status(Z, (value & 0x00FF) == 0x0000);

    if (is_reg_a)
        a = value & 0x00FF;
    else
        bus.write(abs_addr, value & 0x00FF);

    return !is_reg_a;
}

// Branch on Carry Clear
// branch on C = 0
bool C6502::BCC() {
    if (!get_status(C))
        branch();

    return false;
}

// Branch on Carry Set
// branch on C = 1
bool C6502::BCS() {
    if (get_status(C))
        branch();

    return false;
}

// Branch on Result Zero
// branch on Z = 1
bool C6502::BEQ() {
    if (get_status(Z))
        branch();

    return false;
}

// Test Bits in Memory with Accumulator
// bits 7 and 6 of operand are transferred to bit 7 and 6 of SR (N,V);
// the zero-flag is set to the result of operand AND accumulator.
//
// A AND M, M7 -> N, M6 -> V
bool C6502::BIT() {
    uint8_t fetched = fetch();
    set_status(N, (fetched & N) > 0);
    set_status(V, (fetched & V) > 0);
    set_status(Z, (a & fetched) == 0x00);

    return false;
}

// Branch on Result Minus
// branch on N = 1
bool C6502::BMI() {
    if (get_status(N))
        branch();

    return false;
}

// Branch on Result not Zero
// branch on Z = 0
bool C6502::BNE() {
    if (!get_status(Z))
        branch();

    return false;
}

// Branch on Result Plus
// branch on N = 0
bool C6502::BPL() {
    if (!get_status(N))
        branch();

    return false;
}

// Force Break
//
// BRK initiates a software interrupt similar to a hardware
// interrupt (IRQ). The return address pushed to the stack is
// PC+2, providing an extra byte of spacing for a break mark
// (identifying a reason for the break.)
// The status register will be pushed to the stack with the break
// flag set to 1. However, when retrieved during RTI or by a PLP
// instruction, the break flag will be ignored.
// The interrupt disable flag is not set automatically.
//
// interrupt, push PC+2, push SR
bool C6502::BRK() {
    prg_counter += 1;  // +1 already happened after opcode read

    set_status(I, true);
    push(prg_counter >> 8);
    push(prg_counter);

    set_status(B, true);
    push(status);
    set_status(B, false);

    abs_addr = IRQ_ROUTINE_ADDR;
    uint16_t lo = bus.read(abs_addr);
    uint16_t hi = bus.read(abs_addr + 1);
    prg_counter = (hi << 8) | lo;

    return false;
}

// Branch on Overflow Clear
// branch on V = 0
bool C6502::BVC() {
    if (!get_status(V))
        branch();

    return false;
}

// Branch on Overflow Set
// branch on V = 1
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

// Compare Memory with Accumulator
// A - M
bool C6502::CMP() {
    uint16_t fetched = fetch();
    uint16_t diff = ((uint16_t) a) - fetched;
    set_status(C, a >= fetched);
    set_status(Z, (diff & 0x00FF) == 0x0000);
    set_status(N, (diff & 0x0080) > 0);
    return true;
}

// Compare Memory and Index X
// X - M
bool C6502::CPX() {
    uint16_t fetched = fetch();
    uint16_t diff = ((uint16_t) x) - fetched;
    set_status(C, x >= fetched);
    set_status(Z, (diff & 0x00FF) == 0x0000);
    set_status(N, (diff & 0x0080) > 0);
    return true;
}

// Compare Memory and Index Y
// Y - M
bool C6502::CPY() {
    uint16_t fetched = fetch();
    uint16_t diff = ((uint16_t) y) - fetched;
    set_status(C, y >= fetched);
    set_status(Z, (diff & 0x00FF) == 0x0000);
    set_status(N, (diff & 0x0080) > 0);
    return true;
}

// Decrement Memory by One
// M - 1 -> M
bool C6502::DEC() {
    uint8_t value = fetch();
    value--;
    bus.write(abs_addr, value);

    set_status(Z, value == 0x00);
    set_status(N, (value & 0x80) > 0);
    return true;
}

// Decrement Index X by One
// X - 1 -> X
bool C6502::DEX() {
    x--;
    set_status(Z, x == 0x00);
    set_status(N, (x & 0x80) > 0);
    return false;
}

// Decrement Index Y by One
// Y - 1 -> Y
bool C6502::DEY() {
    y--;
    set_status(Z, y == 0x00);
    set_status(N, (y & 0x80) > 0);
    return false;
}

// Exclusive-OR Memory with Accumulator
// A EOR M -> A
bool C6502::EOR() {
    a = a ^ fetch();
    set_status(Z, a == 0x00);
    set_status(N, (a & 0x80) > 0);
    return true;
}

// Increment Memory by One
// M + 1 -> M
bool C6502::INC() {
    uint8_t value = fetch();
    value++;
    bus.write(abs_addr, value);

    set_status(Z, value == 0x00);
    set_status(N, (value & 0x80) > 0);
    return true;
}

// Increment Index X by One
// X + 1 -> X
bool C6502::INX() {
    x++;
    set_status(Z, x == 0x00);
    set_status(N, x & 0x80);
    return false;
}

// Increment Index Y by One
// Y + 1 -> Y
bool C6502::INY() {
    y++;
    set_status(Z, y == 0x00);
    set_status(N, y & 0x80);
    return false;
}

// Jump to New Location
// (PC+1) -> PCL, (PC+2) -> PCH
bool C6502::JMP() {
    prg_counter = abs_addr;
    return false;
}

// Jump to New Location Saving Return Address
// push (PC+2), (PC+1) -> PCL (PC+2) -> PCH
bool C6502::JSR() {
    // JSR instruction is weird: it is a 3-byte instruction and only operates in absolute mode.
    // However, JSR only increments the PC by 2 before pushing it on the stack.
    // Which means the return address points to the last byte of the JSR instruction.
    // Finally, the RTS pops the value from the stack and increments it again before setting the PC to the corrected value.
    // Source: https://retrocomputing.stackexchange.com/questions/19543/why-does-the-6502-jsr-instruction-only-increment-the-return-address-by-2-bytes
    prg_counter--;
    push(prg_counter >> 8);
    push(prg_counter);

    prg_counter = abs_addr;
    return false;
}

// Load Accumulator with Memory
// M -> A
bool C6502::LDA() {
    a = fetch();
    set_status(Z, a == 0x00);
    set_status(N, (a & 0x80) > 0);
    return true;
}

// Load Index X with Memory
// M -> X
bool C6502::LDX() {
    x = fetch();
    set_status(Z, x == 0x00);
    set_status(N, (x & 0x80) > 0);
    return true;
}

// Load Index Y with Memory
// M -> Y
bool C6502::LDY() {
    y = fetch();
    set_status(Z, y == 0x00);
    set_status(N, (y & 0x80) > 0);
    return true;
}

// Shift One Bit Right (Memory or Accumulator)
// 0 -> [76543210] -> C
bool C6502::LSR() {
    bool is_reg_a = (instruction->addrmode.name == "IMP");  // Implied means register A

    uint8_t value = is_reg_a ? a : fetch();
    set_status(C, (value & 0x0001) > 0);
    value = (value >> 1) & 0x7F;
    set_status(N, false);
    set_status(Z, value == 0x00);

    if (is_reg_a)
        a = value;
    else
        bus.write(abs_addr, value);

    return !is_reg_a;
}

// No Operation
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

// OR Memory with Accumulator
// A OR M -> A
bool C6502::ORA() {
    a |= fetch();
    set_status(Z, a == 0x00);
    set_status(N, (a & 0x80) > 0);
    return true;
}

// Push Accumulator on Stack
bool C6502::PHA() {
    push(a);
    return false;
}

// Push Processor Status on Stack
// The status register will be pushed with the break
// flag and bit 5 set to 1.
bool C6502::PHP() {
    push(status | B | U);
    return false;
}

// Pull Accumulator from Stack
bool C6502::PLA() {
    a = pop();
    set_status(Z, a == 0x00);
    set_status(N, (a & 0x80) > 0);
    return false;
}

// Pull Processor Status from Stack
// The status register will be pulled with the break
// flag and bit 5 ignored.
bool C6502::PLP() {
    status = pop();
    return false;
}

// Rotate One Bit Left (Memory or Accumulator)
// C <- [76543210] <- C
bool C6502::ROL() {
    bool is_reg_a = (instruction->addrmode.name == "IMP");  // Implied means register A

    uint16_t value = is_reg_a ? a : fetch();
    value = (value << 1) | (get_status(C) ? 1 : 0);
    set_status(C, (value & 0xFF00) > 0);
    value &= 0x00FF;
    set_status(Z, value == 0x0000);
    set_status(N, (value & 0x0080) > 0);

    if (is_reg_a)
        a = value;
    else
        bus.write(abs_addr, value);

    return !is_reg_a;
}

// Rotate One Bit Right (Memory or Accumulator)
// C -> [76543210] -> C
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

// Return from Interrupt
// The status register is pulled with the break flag
// and bit 5 ignored. Then PC is pulled from the stack.
bool C6502::RTI() {
    status = pop();
    status &= ~U;
    uint16_t lo = pop();
    uint16_t hi = pop();
    prg_counter = (hi << 8) | lo;
    return false;
}

// Return from Subroutine
// pull PC, PC+1 -> PC
bool C6502::RTS() {
    uint16_t lo = pop();
    uint16_t hi = pop();
    prg_counter = (hi << 8) | lo;

    // JSR instruction is weird: it is a 3-byte instruction and only operates in absolute mode.
    // However, JSR only increments the PC by 2 before pushing it on the stack.
    // Which means the return address points to the last byte of the JSR instruction.
    // Finally, the RTS pops the value from the stack and increments it again before setting the PC to the corrected value.
    // Source: https://retrocomputing.stackexchange.com/questions/19543/why-does-the-6502-jsr-instruction-only-increment-the-return-address-by-2-bytes
    prg_counter++;

    return false;
}

// Subtract Memory from Accumulator with Borrow
// A - M - !C -> A
bool C6502::SBC() {
    uint16_t fetched = ((uint16_t) fetch()) ^ 0x00FF;

    uint16_t diff = ((uint16_t) a) + fetched + (get_status(C) ? 1 : 0);

    set_status(C, diff > 0x00FF);
    set_status(Z, (diff & 0x00FF) == 0);
    set_status(N, (diff & 0x8000) > 0);
    set_status(V, (diff ^ (uint16_t) a) & (diff ^ fetched) & 0x0080);

    a = diff & 0x00FF;

    return true;
}

// Set Carry Flag
// 1 -> C
bool C6502::SEC() {
    set_status(C, true);
    return false;
}

// Set Decimal Flag
// 1 -> D
bool C6502::SED() {
    set_status(D, true);
    return false;
}

// Set Interrupt Disable Status
// 1 -> I
bool C6502::SEI() {
    set_status(I, true);
    return false;
}

// Store Accumulator in Memory
// A -> M
bool C6502::STA() {
    bus.write(abs_addr, a);
    return false;
}

// Store Index X in Memory
// X -> M
bool C6502::STX() {
    bus.write(abs_addr, x);
    return false;
}

// Store Index Y in Memory
// Y -> M
bool C6502::STY() {
    bus.write(abs_addr, y);
    return false;
}

// Transfer Accumulator to Index X
// A -> X
bool C6502::TAX() {
    x = a;
    set_status(N, (a & 0x80) > 0);
    set_status(Z, a == 0x00);
    return false;
}

// Transfer Accumulator to Index Y
// A -> Y
bool C6502::TAY() {
    y = a;
    set_status(N, (a & 0x80) > 0);
    set_status(Z, a == 0x00);
    return false;
}

// Transfer Stack Pointer to Index X
// SP -> X
bool C6502::TSX() {
    x = stack_ptr;
    set_status(N, (status & 0x80) > 0);
    set_status(Z, status == 0x00);
    return false;
}

// Transfer Index X to Accumulator
// X -> A
bool C6502::TXA() {
    a = x;
    set_status(N, (x & 0x80) > 0);
    set_status(Z, x == 0x00);
    return false;
}

// Transfer Index X to Stack Register
// X -> SP
bool C6502::TXS() {
    stack_ptr = x;
    return false;
}

// Transfer Index Y to Accumulator
// Y -> A
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