#include "mbc.h"

#include <assert.h>

#include "logger.h"

static void mbc5_init(cartridge_t *cartridge) {
    LOG_DEBUG("Initializing MBC5 state");

    cartridge->state.mbc5.rom_bank = 0x01;
    cartridge->state.mbc5.ram_bank = 0x00;
    cartridge->state.mbc5.ram_enabled = false;
}

static uint8_t mbc5_rom_read(cartridge_t *cartridge, uint16_t addr) {
    if (addr < 0x4000) {
        return cartridge->rom[addr];
    } else if (addr < 0x8000) {
        uint32_t bank = cartridge->state.mbc5.rom_bank & (cartridge->banks_number - 1);
        return cartridge->rom[bank * 0x4000 + (addr - 0x4000)];
    } else {
        assert(0 && "Address out of range for ROM read");
    }

    return 0xFF;
}

static void mbc5_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (addr <= 0x1FFF) {
        cartridge->state.mbc5.ram_enabled = (value & 0x0F) == 0x0A;
    } else if (addr <= 0x2FFF) {
        cartridge->state.mbc5.rom_bank = (cartridge->state.mbc5.rom_bank & 0x0100) | value;
    } else if (addr <= 0x3FFF) {
        cartridge->state.mbc5.rom_bank = (cartridge->state.mbc5.rom_bank & 0x00FF) | ((uint16_t)(value & 0x01) << 8);
    } else if (addr <= 0x5FFF) {
        cartridge->state.mbc5.ram_bank = value & 0x0F;
    } else if (addr >= 0x8000) {
        assert(0 && "Address out of range for ROM write");
    }
}

static uint32_t ram_offset(cartridge_t *cartridge, uint16_t addr) {
    return ((uint32_t)cartridge->state.mbc5.ram_bank * 0x2000 + addr) & (cartridge->ram.size - 1);
}

static uint8_t mbc5_ram_read(cartridge_t *cartridge, uint16_t addr) {
    if (!cartridge->has_ram || !cartridge->state.mbc5.ram_enabled) {
        return 0xFF;
    }
    return cartridge->ram.mem[ram_offset(cartridge, addr)];
}

static void mbc5_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (!cartridge->has_ram || !cartridge->state.mbc5.ram_enabled) {
        return;
    }
    cartridge->ram.mem[ram_offset(cartridge, addr)] = value;
}

const mbc_interface_t mbc5_ops = {
    .init = mbc5_init,
    .rom_read = mbc5_rom_read,
    .rom_write = mbc5_rom_write,
    .ram_read = mbc5_ram_read,
    .ram_write = mbc5_ram_write,
};
