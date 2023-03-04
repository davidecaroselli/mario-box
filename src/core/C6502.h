//
// Created by Davide Caroselli on 04/03/23.
//

#ifndef MARIOBOX_C6502_H
#define MARIOBOX_C6502_H

#include <string>
#include <utility>
#include "SystemBus.h"

class C6502;

struct addrmode_t {
    const std::string name;
    const uint8_t bytes;

    bool (C6502::*f)();

    addrmode_t(std::string name, uint8_t bytes, bool (C6502::*f)())
            : name(std::move(name)), bytes(bytes), f(f) {};

    addrmode_t(const addrmode_t &o) = default;
};

struct inst_t {
    const std::string name;
    const addrmode_t addrmode;
    const uint8_t cycles;

    bool (C6502::*op)();

    inst_t(std::string name, bool (C6502::*op)(), const addrmode_t &addrmode, uint8_t cycles)
            : name(std::move(name)), addrmode(addrmode), cycles(cycles), op(op) {};
};

class C6502 {
public:
    static const uint16_t STACK_BEGIN_ADDR = 0x0100;
    static const uint16_t PROG_BEGIN_ADDR = 0xFFFC;
    static const uint16_t INT_ROUTINE_ADDR = 0xFFFE;

    static const std::vector<inst_t> INSTRUCTIONS;

    enum Status {
        C = 1 << 0,  // Carry bit
        Z = 1 << 1,  // Zero
        I = 1 << 2,  // Disable interrupts
        D = 1 << 3,  // Decimal mode
        B = 1 << 4,  // Break
        U = 1 << 5,  // (unused)
        V = 1 << 6,  // Overflow
        N = 1 << 7,  // Negative
    };

    uint8_t status = 0x00;
    uint8_t a = 0x00;
    uint8_t x = 0x00;
    uint8_t y = 0x00;
    uint16_t prg_counter = 0x0000;
    uint8_t stack_ptr = 0x00;

    SystemBus bus;

    [[nodiscard]] inline bool get_status(Status flag) const {
        return (status & flag) != 0;
    }

    void set_status(Status flag, bool val) {
        if (val)
            status |= flag;
        else
            status &= ~flag;
    }

    C6502() : bus(SystemBus::MAIN_BUS_ID) {};

    const uint8_t cycles() const {
        return cycles_;
    }

    void clock();

    void reset();

    void irq();

    void nmi();

private:
    uint16_t abs_addr = 0x0000;
    uint16_t rel_addr = 0x0000;
    uint8_t opcode = 0x00;
    uint8_t cycles_ = 0;
    inst_t *instruction = nullptr;

    // - Low-level utilities -------------------------------------------------------------------------------------------

    inline uint8_t read_pc() {
        return bus.read(prg_counter++);
    }

    inline void push(uint8_t value) {
        bus.write(STACK_BEGIN_ADDR + stack_ptr--, value);
    }

    inline uint8_t pop() {
        return bus.read(STACK_BEGIN_ADDR + ++stack_ptr);
    }

    static inline bool is_same_page(uint16_t pa, uint16_t pb) {
        return (pa & 0xFF00) == (pb & 0xFF00);
    }

    inline uint8_t fetch() {
        if (instruction->addrmode.name == "IMP") {
            printf("invoked fetch() with Implied Addressing");
            exit(1);
        }

        return bus.read(abs_addr);
    }

    void interrupt();

    void branch();

public:
    // - Addressing modes ----------------------------------------------------------------------------------------------

    bool IMP_(); bool IMM_(); bool REL_();
    bool ZP0_(); bool ZPX_(); bool ZPY_();
    bool ABS_(); bool ABX_(); bool ABY_();
    bool IND_(); bool IZX_(); bool IZY_();

    // - Instructions --------------------------------------------------------------------------------------------------

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

    bool UNK();
};


#endif //MARIOBOX_C6502_H
