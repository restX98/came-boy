#ifndef BUS_H
#define BUS_H

#include "cartridge.h"
#include "mem.h"

typedef struct {
    cartridge_t *cartridge;
    mem_t wram;
    mem_t vram;
    mem_t hram;

    // TODO: Later add other components like RAM, PPU, APU, etc.
} bus_t;

int bus_init(bus_t *bus, cartridge_t *cartridge);
void bus_free(bus_t *bus);

uint8_t bus_read(bus_t *bus, uint16_t addr);

#endif // BUS_H
