//
// Created by Davide Caroselli on 04/03/23.
//

#include "P2C02.h"
#include "Logger.h"

P2C02::P2C02() : bus(SystemBus::PPU_BUS_ID), frame_(Frame::create(256, 240)) {
}

P2C02::~P2C02() {
    delete frame_;
}

uint8_t P2C02::bus_read(uint8_t bus_id, uint16_t addr) {
    uint8_t data = 0;

    if (bus_id == SystemBus::MAIN_BUS_ID) {
        switch (addr) {
            case 0x2000:  // control
            case 0x2001: // mask
                break;
            case 0x2002: // status
                //                           simulating dirty buffer on unused status bits
                data = (status.reg & 0xE0) | (dma_data_buffer & 0x1F);
                status.vertical_blank = 0;
                dma_lsb_addr = false;
                break;
            case 0x2003: // OAM address
            case 0x2004: // OAM data
            case 0x2005: // scroll
                break;
            case 0x2006: // DMA addr
                Logger::err("attempting to read from PPU DMA address");
                break;
            case 0x2007: // DMA data
                data = dma_data_buffer;
                dma_data_buffer = bus.read(dma_addr);
                // no buffered read for palette
                if (dma_addr > 0x3F00) data = dma_data_buffer;

                dma_addr++;
            default:
                Logger::err("invalid PPU address 0x%04X", addr);
                break;
        }
    }

    return data;
}

void P2C02::bus_write(uint8_t bus_id, uint16_t addr, uint8_t val) {
    if (bus_id == SystemBus::MAIN_BUS_ID) {
        switch (addr) {
            case 0x2000:  // control
                control.reg = val;
                break;
            case 0x2001: // mask
                mask.reg = val;
                break;
            case 0x2002: // status
                Logger::warn("(PPU) attempting to write to PPU status");
                break;
            case 0x2003: // OAM address
            case 0x2004: // OAM data
            case 0x2005: // scroll
                break;
            case 0x2006: // DMA addr
                if (dma_lsb_addr)
                    dma_addr = (dma_addr & 0xFF00) | val;
                else
                    dma_addr = (dma_addr & 0x00FF) | (((uint16_t) val) << 8);
                dma_lsb_addr = !dma_lsb_addr;
                break;
            case 0x2007: // DMA data
                bus.write(dma_addr, val);
                dma_addr += 1;
                break;
            default:
                Logger::err("invalid PPU address 0x%04X", addr);
                break;
        }
    }
}

void P2C02::clock() {
    frame_complete = false;

    //if 0 <= scanline && scanline < frame.height && cycle < frame.width  {
    //    frame.set(x: cycle, y: scanline, r: UInt8(drand48() * 255), g: UInt8(drand48() * 255), b: UInt8(drand48() * 255));
    //}

    cycle++;

    if (cycle >= 341) {
        cycle = 0;
        scanline += 1;

        if (scanline >= 261) {
            scanline = -1;
            frame_complete = true;
        }
    }
}
