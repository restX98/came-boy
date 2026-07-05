#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mem.h"

typedef struct cartridge cartridge_t;

typedef struct {
    void (*init)(cartridge_t *cartridge);
    uint8_t(*rom_read)(cartridge_t *cartridge, uint16_t addr);
    void (*rom_write)(cartridge_t *cartridge, uint16_t addr, uint8_t value);
    uint8_t(*ram_read)(cartridge_t *cartridge, uint16_t addr);
    void (*ram_write)(cartridge_t *cartridge, uint16_t addr, uint8_t value);
} mbc_interface_t;

struct cartridge {
    uint8_t *rom;
    size_t size;
    uint16_t banks_number;
    bool has_ram;
    bool has_battery;
    mem_t ram;

    const mbc_interface_t *mbc;   // selected at load time — THE dispatch point

    // MBC register state — union keeps it tidy per-chip
    union {
        struct {
            uint8_t bank1;
            uint8_t bank2;
            uint8_t banking_mode;
            bool ram_enabled;
            bool is_multicart;
        } mbc1;
        struct {
            uint8_t bank;
            bool ram_enabled;
        } mbc2;
        struct {
            uint8_t rom_bank;
            uint8_t ram_bank;
            bool ram_enabled;
        } mbc5;
    } state;

    // header info
    char title[17];
    char destination[32];
    uint8_t version;
    char cartridge_type[35];
};

typedef struct {
    const mbc_interface_t *ops;
    const char *type_name;
    bool has_ram;
    bool has_battery;
} cartridge_type_info_t;

int cartridge_load(cartridge_t *cartridge, const char *filename);

void cartridge_unload(cartridge_t *cartridge);

uint8_t cartridge_rom_read(cartridge_t *cartridge, uint16_t addr);
void cartridge_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value);

uint8_t cartridge_ext_ram_read(cartridge_t *cartridge, uint16_t addr);
void cartridge_ext_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value);

#endif // CARTRIDGE_H
