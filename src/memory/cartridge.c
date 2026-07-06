#include "cartridge.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "mbc/mbc.h"

static const cartridge_type_info_t cartridge_types[256];
static const size_t ext_ram_sizes[6];
static const char *destinations[2];

static bool checksum_verify(uint8_t *rom);

int cartridge_load(cartridge_t *cartridge, const char *filename) {
    LOG_INFO("Loading ROM: %s", filename);

    FILE *ptr = fopen(filename, "rb");
    if (!ptr) {
        LOG_ERROR("Could not open file %s: %s", filename, strerror(errno));
        return -1;
    }

    if (fseek(ptr, 0L, SEEK_END) != 0) {
        LOG_ERROR("Could not seek to end of file %s: %s", filename, strerror(errno));
        fclose(ptr);
        return -1;
    }

    long size = ftell(ptr);
    if (size < 0) {
        LOG_ERROR("Could not determine size of file %s: %s", filename, strerror(errno));
        fclose(ptr);
        return -1;
    }

    LOG_DEBUG("ROM size: %ld bytes", size);

    cartridge->rom = malloc((size_t)size);
    if (!cartridge->rom) {
        LOG_ERROR("Could not allocate %ld bytes for ROM: %s", size, strerror(errno));
        fclose(ptr);
        return -1;
    }
    cartridge->size = (size_t)size;

    rewind(ptr);

    size_t read_size = fread(cartridge->rom, sizeof(*cartridge->rom), cartridge->size, ptr);
    if (read_size != cartridge->size) {
        LOG_ERROR("Could not read file %s: expected %zu bytes, got %zu", filename, cartridge->size, read_size);
        free(cartridge->rom);
        cartridge->rom = NULL;
        cartridge->size = 0;
        fclose(ptr);

        return -1;
    }

    fclose(ptr);

    // 0x0147 — Cartridge type
    cartridge_type_info_t info = cartridge_types[cartridge->rom[0x0147]];
    if (info.ops == NULL) {
        LOG_ERROR("Unsupported cartridge type: 0x%02X", cartridge->rom[0x0147]);
        cartridge_unload(cartridge);

        return -1;
    }

    cartridge->mbc = info.ops;
    cartridge->has_battery = info.has_battery;
    cartridge->has_ram = info.has_ram;
    strncpy(cartridge->cartridge_type, info.type_name, sizeof(cartridge->cartridge_type));

    cartridge->mbc->init(cartridge);

    // 0x0134-0x0143 — Title
    memcpy(&cartridge->title, cartridge->rom + 0x0134, 16);

    // 0x0148 — ROM size
    // 0x52, 0x53 and 0x54 are not handled. Only listed in unofficial docs and are likely inaccurate
    if (cartridge->rom[0x0148] > 8) {
        LOG_ERROR("Unknown ROM size: 0x%02X", cartridge->rom[0x0148]);
        cartridge_unload(cartridge);

        return -1;
    }
    cartridge->banks_number = 1 << (cartridge->rom[0x0148] + 1);

    // 0x014A — Destination code
    strncpy(cartridge->destination, destinations[cartridge->rom[0x014A]], strlen(destinations[cartridge->rom[0x014A]]));

    // 0x014C — Mask ROM version number
    cartridge->version = cartridge->rom[0x014C];

    // 0x014D — Header checksum
    bool is_valid_checksum = checksum_verify(cartridge->rom);
    if (!is_valid_checksum) {
        LOG_ERROR("Wrong checksum");
        cartridge_unload(cartridge);

        return -1;
    }

    // 0x0149 — RAM size
    size_t ext_ram_size = ext_ram_sizes[cartridge->rom[0x0149]];
    if (cartridge->mbc == &mbc2_ops) ext_ram_size = 512;
    if (ext_ram_size != 0 && mem_init(&cartridge->ram, ext_ram_size, "External RAM") != 0) {
        LOG_ERROR("Could not initialize external RAM");
        free(cartridge->rom);
        cartridge->rom = NULL;
        cartridge->size = 0;

        return -1;
    }

    LOG_INFO("ROM loaded successfully");
    LOG_INFO("  Title:       %s", cartridge->title);
    LOG_INFO("  Size:        %zu bytes", cartridge->size);
    LOG_INFO("  MBC:         %s", cartridge->cartridge_type);
    LOG_INFO("  Banks:       %u", cartridge->banks_number);
    LOG_INFO("  Destination: %s", cartridge->destination);
    LOG_INFO("  Version:     %u", cartridge->version);

    return 0;
}

void cartridge_unload(cartridge_t *cartridge) {
    if (!cartridge) {
        LOG_WARN("Attempted to unload a NULL cartridge");
        return;
    }

    LOG_INFO("Unloading cartridge");

    free(cartridge->rom);
    cartridge->rom = NULL;
    cartridge->size = 0;

    mem_free(&cartridge->ram);
}

uint8_t cartridge_rom_read(cartridge_t *cartridge, uint16_t addr) {
    if (!cartridge || !cartridge->mbc) {
        assert(0 && "Attempted to read from ROM with NULL cartridge or MBC");
        return 0xFF;
    }

    return cartridge->mbc->rom_read(cartridge, addr);
}

void cartridge_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (!cartridge || !cartridge->mbc) {
        assert(0 && "Attempted to write to ROM with NULL cartridge or MBC");
        return;
    }

    cartridge->mbc->rom_write(cartridge, addr, value);
}

uint8_t cartridge_ext_ram_read(cartridge_t *cartridge, uint16_t addr) {
    if (!cartridge || !cartridge->mbc) {
        assert(0 && "Attempted to read from external RAM with NULL cartridge or MBC");
        return 0xFF;
    }

    return cartridge->mbc->ram_read(cartridge, addr);
}

void cartridge_ext_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (!cartridge || !cartridge->mbc) {
        assert(0 && "Attempted to write to external RAM with NULL cartridge or MBC");
        return;
    }

    cartridge->mbc->ram_write(cartridge, addr, value);
}

static bool checksum_verify(uint8_t *rom) {
    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - rom[address] - 1;
    }

    return checksum == rom[0x014D];
}

static const cartridge_type_info_t cartridge_types[256] = {
    [0x00] = {&no_mbc_ops, "ROM ONLY", false, false},
    [0x01] = {&mbc1_ops, "MBC1", false, false},
    [0x02] = {&mbc1_ops, "MBC1+RAM", true, false},
    [0x03] = {&mbc1_ops, "MBC1+RAM+BATTERY", true, true},
    [0x05] = {&mbc2_ops, "MBC2", false, false},
    [0x06] = {&mbc2_ops, "MBC2+BATTERY", false, true},
    [0x08] = {NULL, "ROM+RAM", true, false},
    [0x09] = {NULL, "ROM+RAM+BATTERY", true, true},
    [0x0B] = {NULL, "MMM01", false, false},
    [0x0C] = {NULL, "MMM01+RAM", true, false},
    [0x0D] = {NULL, "MMM01+RAM+BATTERY", true, true},
    [0x0F] = {NULL, "MBC3+TIMER+BATTERY", false, true},
    [0x10] = {NULL, "MBC3+TIMER+RAM+BATTERY", true, true},
    [0x11] = {NULL, "MBC3", false, false},
    [0x12] = {NULL, "MBC3+RAM", true, false},
    [0x13] = {NULL, "MBC3+RAM+BATTERY", true, true},
    [0x19] = {&mbc5_ops, "MBC5", false, false},
    [0x1A] = {&mbc5_ops, "MBC5+RAM", true, false},
    [0x1B] = {&mbc5_ops, "MBC5+RAM+BATTERY", true, true},
    [0x1C] = {&mbc5_ops, "MBC5+RUMBLE", false, false},
    [0x1D] = {&mbc5_ops, "MBC5+RUMBLE+RAM", true, false},
    [0x1E] = {&mbc5_ops, "MBC5+RUMBLE+RAM+BATTERY", true, true},
    [0x20] = {NULL, "MBC6", false, false},
    [0x22] = {NULL, "MBC7+SENSOR+RUMBLE+RAM+BATTERY", true, true},
    [0xFC] = {NULL, "POCKET CAMERA", false, false},
    [0xFD] = {NULL, "BANDAI TAMA5", false, false},
    [0xFE] = {NULL, "HuC3", false, false},
    [0xFF] = {NULL, "HuC1+RAM+BATTERY", true, true},
};

static const size_t ext_ram_sizes[6] = {
    [0x00] = 0,
    [0x01] = 0,
    [0x02] = 8 * 1024,
    [0x03] = 32 * 1024,
    [0x04] = 128 * 1024,
    [0x05] = 64 * 1024,
};

static const char *destinations[2] = {
    [0x00] = "Japan (and possibly overseas)",
    [0x01] = "Overseas only",
};
