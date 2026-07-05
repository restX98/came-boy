#include "mbc.h"

#include <assert.h>

#include "logger.h"

static void mbc2_init(cartridge_t *cartridge) {
    LOG_DEBUG("Initializing MBC2 state");

    cartridge->state.mbc2.bank = 0x01;
    cartridge->state.mbc2.ram_enabled = false;
}

static uint8_t mbc2_rom_read(cartridge_t *cartridge, uint16_t addr) {
    if (addr < 0x4000) {
        return cartridge->rom[addr];
    } else if (addr < 0x8000) {
        uint8_t bank = cartridge->state.mbc2.bank & (cartridge->banks_number - 1);
        addr -= 0x4000;
        return cartridge->rom[bank * 0x4000 + addr];
    } else {
        assert(0 && "Address out of range for ROM read");
    }

    return 0xFF;
}

static void mbc2_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (addr < 0x4000) {
        if ((addr & 0x0100) == 0) {
            cartridge->state.mbc2.ram_enabled = (value & 0x0F) == 0x0A;
        } else {
            uint8_t bank = value & 0x0F;
            if (bank == 0x00) {
                bank = 0x01;
            }
            cartridge->state.mbc2.bank = bank;
        }
    } else if (addr >= 0x8000) {
        assert(0 && "Address out of range for ROM write");
    }
}

static uint8_t mbc2_ram_read(cartridge_t *cartridge, uint16_t addr) {
    if (!cartridge->state.mbc2.ram_enabled) {
        return 0xFF;
    }
    return cartridge->ram.mem[addr & 0x01FF] | 0xF0;
}

static void mbc2_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (!cartridge->state.mbc2.ram_enabled) {
        return;
    }
    cartridge->ram.mem[addr & 0x01FF] = value & 0x0F;
}

const mbc_interface_t mbc2_ops = {
    .init = mbc2_init,
    .rom_read = mbc2_rom_read,
    .rom_write = mbc2_rom_write,
    .ram_read = mbc2_ram_read,
    .ram_write = mbc2_ram_write,
};
