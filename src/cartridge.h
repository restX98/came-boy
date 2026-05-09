#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#define EXT_RAM_SIZE (8 * 1024) // 8KB of External RAM

#include <stdint.h>
#include <stdlib.h>

#include "mem.h"

typedef struct {
    // TODO: eventually make it a mem_t(?)
    uint8_t *rom;
    size_t size;
    uint8_t bank;

    mem_t ext_ram;
} cartridge_t;

int cartridge_load(cartridge_t *cartridge, const char *filename);

void cartridge_unload(cartridge_t *cartridge);

#endif // CARTRIDGE_H
