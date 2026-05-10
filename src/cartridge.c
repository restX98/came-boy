#include "cartridge.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

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
    cartridge->bank = 1;

    rewind(ptr);

    size_t read_size = fread(cartridge->rom, sizeof(*cartridge->rom), cartridge->size, ptr);
    if (read_size != cartridge->size) {
        LOG_ERROR("Could not read file %s: expected %zu bytes, got %zu", filename, cartridge->size, read_size);
        free(cartridge->rom);
        cartridge->rom = NULL;
        cartridge->size = 0;
        cartridge->bank = 0;
        fclose(ptr);

        return -1;
    }

    fclose(ptr);

    // 0x0149 — RAM size
    size_t ext_ram_size = ext_ram_sizes[cartridge->rom[0x0149]];
    if (ext_ram_size != 0 && mem_init(&cartridge->ext_ram, ext_ram_size, "External RAM") != 0) {
        LOG_ERROR("Could not initialize external RAM");
        free(cartridge->rom);
        cartridge->rom = NULL;
        cartridge->size = 0;
        cartridge->bank = 0;

        return -1;
    }

    // 0x0134-0x0143 — Title
    memcpy(&cartridge->title, cartridge->rom + 0x0134, 16);

    // 0x0147 — Cartridge type
    cartridge->mbc = mbc_types[cartridge->rom[0x0147]];
    if (cartridge->mbc.name[0] == '\0') {
        LOG_ERROR("Unknown MBC type: 0x%02X", cartridge->rom[0x0147]);
        cartridge_unload(cartridge);

        return -1;
    }

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

    LOG_INFO("ROM loaded successfully");
    LOG_INFO("  Title:       %s", cartridge->title);
    LOG_INFO("  Size:        %zu bytes", cartridge->size);
    LOG_INFO("  MBC:         %s", cartridge->mbc.name);
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
    cartridge->bank = 0;

    mem_free(&cartridge->ext_ram);
}


uint8_t cartridge_rom_read(cartridge_t *cartridge, uint16_t addr) {
    return cartridge->rom[addr];
}

uint8_t cartridge_ext_ram_read(cartridge_t *cartridge, uint16_t addr) {
    if (!cartridge->mbc.hasRam) {
        return 0xFF;
    }
    return cartridge->ext_ram.mem[addr];
}
void cartridge_ext_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (cartridge->mbc.hasRam) {
        cartridge->ext_ram.mem[addr] = value;
    }
}

static bool checksum_verify(uint8_t *rom) {
    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - rom[address] - 1;
    }

    return checksum == rom[0x014D];
}

const mbc_t mbc_types[256] = {
    [0x00] = {0x00, "ROM ONLY", false},
    [0x01] = {0x01, "MBC1", false},
    [0x02] = {0x02, "MBC1+RAM", true},
    [0x03] = {0x03, "MBC1+RAM+BATTERY", true},
    [0x05] = {0x05, "MBC2", false},
    [0x06] = {0x06, "MBC2+BATTERY", false},
    [0x08] = {0x08, "ROM+RAM", true},
    [0x09] = {0x09, "ROM+RAM+BATTERY", true},
    [0x0B] = {0x0B, "MMM01", false},
    [0x0C] = {0x0C, "MMM01+RAM", true},
    [0x0D] = {0x0D, "MMM01+RAM+BATTERY", true},
    [0x0F] = {0x0F, "MBC3+TIMER+BATTERY", false},
    [0x10] = {0x10, "MBC3+TIMER+RAM+BATTERY", true},
    [0x11] = {0x11, "MBC3", false},
    [0x12] = {0x12, "MBC3+RAM", true},
    [0x13] = {0x13, "MBC3+RAM+BATTERY", true},
    [0x19] = {0x19, "MBC5", false},
    [0x1A] = {0x1A, "MBC5+RAM", true},
    [0x1B] = {0x1B, "MBC5+RAM+BATTERY", true},
    [0x1C] = {0x1C, "MBC5+RUMBLE", false},
    [0x1D] = {0x1D, "MBC5+RUMBLE+RAM", true},
    [0x1E] = {0x1E, "MBC5+RUMBLE+RAM+BATTERY", true},
    [0x20] = {0x20, "MBC6", false},
    [0x22] = {0x22, "MBC7+SENSOR+RUMBLE+RAM+BATTERY", true},
    [0xFC] = {0xFC, "POCKET CAMERA", false},
    [0xFD] = {0xFD, "BANDAI TAMA5", false},
    [0xFE] = {0xFE, "HuC3", false},
    [0xFF] = {0xFF, "HuC1+RAM+BATTERY", true},
};

const size_t ext_ram_sizes[6] = {
    [0x00] = 0,
    [0x01] = 0,
    [0x02] = 8 * 1024,
    [0x03] = 32 * 1024,
    [0x04] = 128 * 1024,
    [0x05] = 64 * 1024,
};

const char *destinations[2] = {
    [0x00] = "Japan (and possibly overseas)",
    [0x01] = "Overseas only",
};
