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

typedef struct {
    char name[30];
    uint16_t start;
    uint16_t end;
    uint8_t(*read_fn)(bus_t *bus, uint16_t addr);
} mem_region_t;

int bus_init(bus_t *bus, cartridge_t *cartridge);
void bus_free(bus_t *bus);

uint8_t bus_read(bus_t *bus, uint16_t addr);

// TODO: Implement bus_write and corresponding write functions for each memory region

#endif // BUS_H
