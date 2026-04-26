#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>

typedef struct {
    uint8_t *rom;
    uint32_t size;
    uint8_t bank;
} cartridge_t;

int cartridge_load(cartridge_t *cartridge, const char *filename);

void cartridge_unload(cartridge_t *cartridge);

#endif // CARTRIDGE_H
