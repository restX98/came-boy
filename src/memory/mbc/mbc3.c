#include "mbc.h"

#include <assert.h>
#include <time.h>

#include "logger.h"

// RTC register indices into state.mbc3.rtc / rtc_latched.
enum {
    RTC_SECONDS = 0,
    RTC_MINUTES = 1,
    RTC_HOURS = 2,
    RTC_DAY_LOW = 3,
    RTC_DAY_HIGH = 4,
};

// Bits of the day-high (0x0C) register.
#define RTC_DH_DAY_MSB 0x01  // bit 8 of the 9-bit day counter
#define RTC_DH_HALT    0x40  // when set, the clock is stopped
#define RTC_DH_CARRY   0x80  // set (and sticky) when the day counter overflows

static void mbc3_init(cartridge_t *cartridge) {
    LOG_DEBUG("Initializing MBC3 state");

    cartridge->state.mbc3.rom_bank = 0x01;
    cartridge->state.mbc3.ram_bank = 0x00;
    cartridge->state.mbc3.ram_enabled = false;
    cartridge->state.mbc3.latch = 0xFF;

    for (int i = 0; i < 5; i++) {
        cartridge->state.mbc3.rtc[i] = 0;
        cartridge->state.mbc3.rtc_latched[i] = 0;
    }
    cartridge->state.mbc3.rtc_last = time(NULL);

    // Only the +TIMER cartridge types (0x0F, 0x10) carry an RTC.
    uint8_t type = cartridge->rom[0x0147];
    cartridge->state.mbc3.has_rtc = (type == 0x0F || type == 0x10);
    if (cartridge->state.mbc3.has_rtc) {
        LOG_INFO("MBC3 real-time clock present");
    }
}

// Advance the live RTC counters by the real time elapsed since the last update.
// The clock does not tick while halted.
static void rtc_tick(cartridge_t *cartridge) {
    uint8_t *rtc = cartridge->state.mbc3.rtc;
    time_t now = time(NULL);

    if (rtc[RTC_DAY_HIGH] & RTC_DH_HALT) {
        cartridge->state.mbc3.rtc_last = now;
        return;
    }

    int64_t elapsed = (int64_t)(now - cartridge->state.mbc3.rtc_last);
    cartridge->state.mbc3.rtc_last = now;
    if (elapsed <= 0) {
        return;
    }

    int64_t seconds = rtc[RTC_SECONDS] + elapsed;
    rtc[RTC_SECONDS] = (uint8_t)(seconds % 60);
    int64_t minutes = rtc[RTC_MINUTES] + seconds / 60;
    rtc[RTC_MINUTES] = (uint8_t)(minutes % 60);
    int64_t hours = rtc[RTC_HOURS] + minutes / 60;
    rtc[RTC_HOURS] = (uint8_t)(hours % 24);

    int64_t days = (((rtc[RTC_DAY_HIGH] & RTC_DH_DAY_MSB) << 8) | rtc[RTC_DAY_LOW]) + hours / 24;
    rtc[RTC_DAY_LOW] = (uint8_t)(days & 0xFF);
    uint8_t day_high = rtc[RTC_DAY_HIGH] & (RTC_DH_HALT | RTC_DH_CARRY);
    day_high |= (days >> 8) & RTC_DH_DAY_MSB;
    if (days > 0x1FF) {
        day_high |= RTC_DH_CARRY;  // sticky until cleared by software
    }
    rtc[RTC_DAY_HIGH] = day_high;
}

static uint8_t mbc3_rom_read(cartridge_t *cartridge, uint16_t addr) {
    if (addr < 0x4000) {
        return cartridge->rom[addr];
    } else if (addr < 0x8000) {
        uint32_t bank = cartridge->state.mbc3.rom_bank & (cartridge->banks_number - 1);
        return cartridge->rom[bank * 0x4000 + (addr - 0x4000)];
    } else {
        assert(0 && "Address out of range for ROM read");
    }

    return 0xFF;
}

static void mbc3_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (addr <= 0x1FFF) {
        // RAM and RTC register enable.
        cartridge->state.mbc3.ram_enabled = (value & 0x0F) == 0x0A;
    } else if (addr <= 0x3FFF) {
        // 7-bit ROM bank number; bank 0 is remapped to bank 1.
        uint8_t bank = value & 0x7F;
        if (bank == 0x00) {
            bank = 0x01;
        }
        cartridge->state.mbc3.rom_bank = bank;
    } else if (addr <= 0x5FFF) {
        // 0x00-0x03 select a RAM bank; 0x08-0x0C select an RTC register.
        cartridge->state.mbc3.ram_bank = value;
    } else if (addr <= 0x7FFF) {
        // Latch clock data: a 0x00 followed by 0x01 snapshots the live counters.
        if (cartridge->state.mbc3.latch == 0x00 && value == 0x01) {
            rtc_tick(cartridge);
            for (int i = 0; i < 5; i++) {
                cartridge->state.mbc3.rtc_latched[i] = cartridge->state.mbc3.rtc[i];
            }
        }
        cartridge->state.mbc3.latch = value;
    } else {
        assert(0 && "Address out of range for ROM write");
    }
}

static uint8_t mbc3_ram_read(cartridge_t *cartridge, uint16_t addr) {
    if (!cartridge->state.mbc3.ram_enabled) {
        return 0xFF;
    }

    uint8_t bank = cartridge->state.mbc3.ram_bank;
    if (bank <= 0x03) {
        if (!cartridge->has_ram) {
            return 0xFF;
        }
        uint32_t offset = ((uint32_t)bank * 0x2000 + addr) & (cartridge->ram.size - 1);
        return cartridge->ram.mem[offset];
    }
    if (bank >= 0x08 && bank <= 0x0C) {
        if (!cartridge->state.mbc3.has_rtc) {
            return 0xFF;
        }
        return cartridge->state.mbc3.rtc_latched[bank - 0x08];
    }

    return 0xFF;
}

static void mbc3_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (!cartridge->state.mbc3.ram_enabled) {
        return;
    }

    uint8_t bank = cartridge->state.mbc3.ram_bank;
    if (bank <= 0x03) {
        if (!cartridge->has_ram) {
            return;
        }
        uint32_t offset = ((uint32_t)bank * 0x2000 + addr) & (cartridge->ram.size - 1);
        cartridge->ram.mem[offset] = value;
        return;
    }
    if (bank >= 0x08 && bank <= 0x0C && cartridge->state.mbc3.has_rtc) {
        // Writing a counter also stops it drifting: re-anchor the tick baseline.
        rtc_tick(cartridge);
        uint8_t reg = bank - 0x08;
        if (reg == RTC_DAY_HIGH) {
            value &= (RTC_DH_DAY_MSB | RTC_DH_HALT | RTC_DH_CARRY);
        }
        cartridge->state.mbc3.rtc[reg] = value;
        cartridge->state.mbc3.rtc_latched[reg] = value;
    }
}

const mbc_interface_t mbc3_ops = {
    .init = mbc3_init,
    .rom_read = mbc3_rom_read,
    .rom_write = mbc3_rom_write,
    .ram_read = mbc3_ram_read,
    .ram_write = mbc3_ram_write,
};
