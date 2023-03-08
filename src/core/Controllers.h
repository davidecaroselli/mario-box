//
// Created by Davide Caroselli on 08/03/23.
//

#ifndef MARIOBOX_CONTROLLERS_H
#define MARIOBOX_CONTROLLERS_H


#include "SystemBus.h"

enum Button {
    A      = (1 << 7),
    B      = (1 << 6),
    Select = (1 << 5),
    Start  = (1 << 4),
    Up     = (1 << 3),
    Down   = (1 << 2),
    Left   = (1 << 1),
    Right  = (1 << 0)
};

class Controllers : public SBDevice {
public:
    inline void button(uint8_t controller, Button button, bool pressed) {
        if (pressed)
            buttons[controller & 0x01] |= button;
        else
            buttons[controller & 0x01] &= ~button;
    }

    uint16_t bus_begin(uint8_t bus_id) override {
        return 0x4016;
    }

    uint16_t bus_end(uint8_t bus_id) override {
        return 0x4017;
    }

    uint8_t bus_read(uint8_t bus_id, uint16_t addr) override;

    void bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) override;

private:
    uint8_t buttons[2] = {0, 0};
    uint8_t buffer[2] = {0, 0};

};


#endif //MARIOBOX_CONTROLLERS_H
