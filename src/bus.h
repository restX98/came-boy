#ifndef BUS_H
#define BUS_H

#include "io_reg.h"
#include "memory/cartridge.h"
#include "memory/mem.h"

typedef struct bus_s {
    cartridge_t *cartridge;
    mem_t wram;
    mem_t vram;
    mem_t hram;

    mem_t oam; // Object Attribute Memory (OAM) for sprite data

    bool vram_accessible;
    bool oam_accessible;

    io_reg_t io_reg;
} bus_t;

typedef struct {
    char name[30];
    uint16_t start;
    uint16_t end;
    uint8_t(*read_fn)(bus_t *bus, uint16_t addr);
    void(*write_fn)(bus_t *bus, uint16_t addr, uint8_t value);
} mem_region_t;

int bus_init(bus_t *bus, cartridge_t *cartridge);
void bus_free(bus_t *bus);

uint8_t bus_read(bus_t *bus, uint16_t addr);
void bus_write(bus_t *bus, uint16_t addr, uint8_t value);

#endif // BUS_H
