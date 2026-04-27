#ifndef BUS_H
#define BUS_H

#include "cartridge.h"

typedef struct {
    cartridge_t *cartridge;
    // TODO: Later add other components like RAM, PPU, APU, etc.
} bus_t;

void bus_init(bus_t *bus, cartridge_t *cartridge);

uint8_t bus_read(bus_t *bus, uint16_t addr);

#endif // BUS_H
