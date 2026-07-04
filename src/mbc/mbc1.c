#include "mbc.h"

#include <assert.h>
#include <string.h>

#include "logger.h"

static const uint8_t nintendo_logo[48] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

static void mbc1_init(cartridge_t *cartridge) {
    LOG_DEBUG("Resetting MBC1 state");

    cartridge->state.mbc1.bank1 = 0x01;
    cartridge->state.mbc1.bank2 = 0x00;
    cartridge->state.mbc1.banking_mode = 0;
    cartridge->state.mbc1.ram_enabled = false;


    cartridge->state.mbc1.is_multicart = false;

    // MBC1 multicart detection (header can't distinguish it so use logo heuristic)
    if (cartridge->size == 0x100000) {
        if (memcmp(cartridge->rom + 0x10 * 0x4000 + 0x0104,
            nintendo_logo, sizeof(nintendo_logo)) == 0) {
            cartridge->state.mbc1.is_multicart = true;
            LOG_INFO("MBC1 multicart detected");
        }
    }
}

static uint8_t mbc1_rom_read(cartridge_t *cartridge, uint16_t addr) {
    uint32_t bank;

    if (addr < 0x4000) {
        uint8_t shift = cartridge->state.mbc1.is_multicart ? 4 : 5;
        bank = (cartridge->state.mbc1.banking_mode == 1) ? (cartridge->state.mbc1.bank2 << shift) : 0;
    } else if (addr < 0x8000) {
        if (cartridge->state.mbc1.is_multicart) {
            bank = (cartridge->state.mbc1.bank1 & 0x0F) | (cartridge->state.mbc1.bank2 << 4);
        } else {
            bank = cartridge->state.mbc1.bank1 | (cartridge->state.mbc1.bank2 << 5);
        }
        addr -= 0x4000;
    } else {
        assert(0 && "Address out of range for ROM read");
        return 0xFF;
    }

    bank &= cartridge->banks_number - 1;
    return cartridge->rom[bank * 0x4000 + addr];
}

static void mbc1_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (addr <= 0x1FFF) {
        cartridge->state.mbc1.ram_enabled = (value & 0x0F) == 0x0A;
    } else if (addr <= 0x3FFF) {
        uint8_t bank1 = value & 0x1F;
        if (bank1 == 0x00) {
            bank1 = 0x01;
        }
        cartridge->state.mbc1.bank1 = bank1;
    } else if (addr <= 0x5FFF) {
        cartridge->state.mbc1.bank2 = value & 0x03;
    } else if (addr <= 0x7FFF) {
        // Banking Mode Select
        cartridge->state.mbc1.banking_mode = value & 0x01;
    }
}

static uint32_t ram_offset(cartridge_t *cartridge, uint16_t addr) {
    uint8_t bank = (cartridge->state.mbc1.banking_mode == 1) ? cartridge->state.mbc1.bank2 : 0;
    return ((uint32_t)bank * 0x2000 + addr) & (cartridge->ram.size - 1);
}

static uint8_t mbc1_ram_read(cartridge_t *cartridge, uint16_t addr) {
    if (!cartridge->has_ram || !cartridge->state.mbc1.ram_enabled) {
        return 0xFF;
    }
    return cartridge->ram.mem[ram_offset(cartridge, addr)];
}

static void mbc1_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (!cartridge->has_ram || !cartridge->state.mbc1.ram_enabled) {
        return;
    }
    cartridge->ram.mem[ram_offset(cartridge, addr)] = value;
}

const mbc_interface_t mbc1_ops = {
    .init = mbc1_init,
    .rom_read = mbc1_rom_read,
    .rom_write = mbc1_rom_write,
    .ram_read = mbc1_ram_read,
    .ram_write = mbc1_ram_write,
};
