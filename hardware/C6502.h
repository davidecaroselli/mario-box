//
// Created by Davide Caroselli on 25/02/23.
//

#ifndef NESSY_C6502_H
#define NESSY_C6502_H

#include <cstdint>
#include <utility>
#include "hardware/bus/Bus.h"

#define STACK_BEGIN (0x0100)
#define PROG_BEGIN_PTR (0xFFFC)
#define INT_ROUTINE_PTR (0xFFFE)

class C6502 {
public:
    explicit C6502(std::shared_ptr<Bus> bus);

    uint8_t a = 0x00;       // Accumulator register
    uint8_t x = 0x00;       // X register
    uint8_t y = 0x00;       // Y register
    uint8_t stackp = 0x00;  // Stack pointer
    uint16_t pc = 0x0000;   // Program counter
    uint8_t status = 0x00;  // Status register

    enum flag_t {
        C = (1 << 0),  // Carry bit
        Z = (1 << 1),  // Zero
        I = (1 << 2),  // Disable interrupts
        D = (1 << 3),  // Decimal mode
        B = (1 << 4),  // Break
        U = (1 << 5),  // Unused
        V = (1 << 6),  // Overflow
        N = (1 << 7)   // Negative
    };

private:
    bool getf(flag_t flag) const;

    void setf(flag_t flag, bool value);

public:  // debug tools

    void step();

    void printf() const;

public:
    void clock();

    void reset();

    void irq();

    void nmi();

private:
    struct instruction_t {
        std::string name = "???";
        bool(C6502::*run)() = nullptr;
        bool(C6502::*addrmode)() = nullptr;
        uint8_t cycles = 0;
    };

    std::shared_ptr<Bus> bus;

    void interrupt();

    uint8_t fetch();

    uint16_t addr_abs = 0x0000;
    uint16_t addr_rel = 0x0000;
    uint8_t opcode = 0x00;
    instruction_t inst = {"???", nullptr, &C6502::IMP, 0};
    uint8_t cycles = 0;

private:
    // Addressing modes - returns number of additional clock cycles (if needed)
    bool IMP(); bool IMM(); bool REL();
    bool ZP0(); bool ZPX(); bool ZPY();
    bool ABS(); bool ABX(); bool ABY();
    bool IND(); bool IZX(); bool IZY();

    // Instructions
    std::vector<instruction_t> instructions;

    void push(uint8_t value);

    uint8_t pop();

    void branch();

    bool ADC();	bool AND();	bool ASL();	bool BCC();
    bool BCS();	bool BEQ();	bool BIT();	bool BMI();
    bool BNE();	bool BPL();	bool BRK();	bool BVC();
    bool BVS();	bool CLC();	bool CLD();	bool CLI();
    bool CLV();	bool CMP();	bool CPX();	bool CPY();
    bool DEC();	bool DEX();	bool DEY();	bool EOR();
    bool INC();	bool INX();	bool INY();	bool JMP();
    bool JSR();	bool LDA();	bool LDX();	bool LDY();
    bool LSR();	bool NOP();	bool ORA();	bool PHA();
    bool PHP();	bool PLA();	bool PLP();	bool ROL();
    bool ROR();	bool RTI();	bool RTS();	bool SBC();
    bool SEC();	bool SED();	bool SEI();	bool STA();
    bool STX();	bool STY();	bool TAX();	bool TAY();
    bool TSX();	bool TXA();	bool TXS();	bool TYA();

};

class cpu_error: public std::exception {
public:
    explicit cpu_error(std::string message): message(std::move(message)) {};

    const char *what() const _NOEXCEPT override {
        return message.c_str();
    }

private:
    const std::string message;
};

#endif //NESSY_C6502_H
