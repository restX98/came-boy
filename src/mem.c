#include "mem.h"

#include <stdlib.h>
#include <string.h>

#include "logger.h"

int mem_init(mem_t *memory, size_t size, const char *name) {
    LOG_INFO("Initializing %s memory, size: %zu", name, size);

    strncpy(memory->name, name, sizeof(memory->name) - 1);
    memory->name[sizeof(memory->name) - 1] = '\0';

    memory->mem = malloc(size);
    if (!memory->mem) {
        LOG_ERROR("Could not allocate memory for %s", memory->name);
        return -1;
    }
    memory->size = size;

    return 0;
}


void mem_free(mem_t *memory) {
    if (!memory) {
        LOG_WARN("Attempted to free a NULL memory");
        return;
    }

    LOG_INFO("Freeing %s memory", memory->name);

    free(memory->mem);
    memory->mem = NULL;
    memory->size = 0;
}
