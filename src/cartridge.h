#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mem.h"

typedef struct {
    uint8_t code;
    char name[32];
    bool hasRam;
} mbc_t;

typedef struct {
    // TODO: eventually make it a mem_t(?)
    uint8_t *rom;
    size_t size;
    uint8_t bank;
    uint8_t banks_number;

    char title[16];
    mbc_t mbc;
    char destination[32];
    uint8_t version;

    mem_t ext_ram;
} cartridge_t;

int cartridge_load(cartridge_t *cartridge, const char *filename);

void cartridge_unload(cartridge_t *cartridge);

uint8_t cartridge_rom_read(cartridge_t *cartridge, uint16_t addr);
uint8_t cartridge_ext_ram_read(cartridge_t *cartridge, uint16_t addr);
void cartridge_ext_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value);

extern const mbc_t mbc_types[256];
extern const size_t ext_ram_sizes[6];
extern const char *destinations[2];

#endif // CARTRIDGE_H
