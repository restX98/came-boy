#include "cartridge.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

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

    if (mem_init(&cartridge->ext_ram, EXT_RAM_SIZE, "External RAM") != 0) {
        LOG_ERROR("Could not initialize external RAM");
        free(cartridge->rom);
        cartridge->rom = NULL;
        cartridge->size = 0;
        cartridge->bank = 0;

        return -1;
    }

    LOG_INFO("ROM loaded successfully: %zu bytes", cartridge->size);

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
