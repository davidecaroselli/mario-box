//
// Created by Davide Caroselli on 04/03/23.
//

#include "SystemBus.h"
#include "Logger.h"

using namespace std;

void SystemBus::connect(SBDevice *device) {
    devices.push_back(device);
}

void SystemBus::disconnect(SBDevice *device) {
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if ((*it) == device) {
            devices.erase(it);
            break;
        }
    }
}

uint8_t SystemBus::read(uint16_t addr) {
    for (auto &device : devices) {
        if (device->bus_begin(id) <= addr && addr <= device->bus_end(id)) {
            return device->bus_read(id, addr);
        }
    }

    Logger::warn("SystemBus(%d): read, no device found at 0x%04X", id, addr);
    return 0x00;
}

void SystemBus::write(uint16_t addr, uint8_t val) {
    for (auto &device : devices) {
        if (device->bus_begin(id) <= addr && addr <= device->bus_end(id)) {
            device->bus_write(id, addr, val);
            return;
        }
    }

    Logger::warn("SystemBus(%d): write, no device found at 0x%04X", id, addr);
}