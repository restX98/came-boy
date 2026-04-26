#include "cartridge.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

    cartridge->size = (uint32_t)size;
    cartridge->bank = 1;
    cartridge->rom = malloc(size);
    if (!cartridge->rom) {
        LOG_ERROR("Could not allocate %ld bytes for ROM: %s", size, strerror(errno));
        fclose(ptr);
        return -1;
    }

    rewind(ptr);

    size_t read_size = fread(cartridge->rom, sizeof(*cartridge->rom), cartridge->size, ptr);
    if (read_size != cartridge->size) {
        LOG_ERROR("Could not read file %s: expected %u bytes, got %zu", filename, cartridge->size, read_size);
        free(cartridge->rom);
        fclose(ptr);
        return -1;
    }

    fclose(ptr);
    LOG_INFO("ROM loaded successfully: %u bytes", cartridge->size);
    return 0;
}

void cartridge_unload(cartridge_t *cartridge) {
    if (cartridge->rom) {
        free(cartridge->rom);
        cartridge->rom = NULL;
        cartridge->size = 0;
        cartridge->bank = 0;
    }
}
