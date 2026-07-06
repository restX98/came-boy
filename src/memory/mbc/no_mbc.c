#include "mbc.h"

#include <assert.h>

#include "logger.h"

static void no_mbc_init(cartridge_t *cartridge) {
    (void)cartridge;
    // No MBC has no state to reset
}

static uint8_t no_mbc_rom_read(cartridge_t *cartridge, uint16_t addr) {
    if (addr < 0x8000) {
        return cartridge->rom[addr];
    } else {
        LOG_WARN("Attempted to read from ROM address: 0x%04X - No MBC supports only 0x0000-0x7FFF", addr);
        return 0xFF; // Return 0xFF for out-of-range addresses
    }
}

static void no_mbc_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    (void)cartridge;
    (void)addr;
    (void)value;
    // No-op for no-MBC
    LOG_WARN("Attempted to write to ROM address: 0x%04X with value: 0x%02X - No MBC supports only read-only ROM", addr, value);
}

static uint8_t no_mbc_ram_read(cartridge_t *cartridge, uint16_t addr) {
    (void)cartridge;
    (void)addr;
    LOG_WARN("Attempted to read from RAM address: 0x%04X - No MBC supports only ROM", addr);
    return 0xFF; // No external RAM, return 0xFF
}

static void no_mbc_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    (void)cartridge;
    (void)addr;
    (void)value;
    LOG_WARN("Attempted to write to RAM address: 0x%04X with value: 0x%02X - No MBC supports only ROM", addr, value);
}

const mbc_interface_t no_mbc_ops = {
    .init = no_mbc_init,
    .rom_read = no_mbc_rom_read,
    .rom_write = no_mbc_rom_write,
    .ram_read = no_mbc_ram_read,
    .ram_write = no_mbc_ram_write,
};
