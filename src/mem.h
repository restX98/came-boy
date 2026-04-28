#ifndef MEM_H
#define MEM_H

#define WRAM_SIZE (8 * 1024) // 8KB of WRAM
#define VRAM_SIZE (8 * 1024) // 8KB of VRAM
#define HRAM_SIZE 127 // 127 bytes of HRAM

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *mem;
    size_t size;
    char name[16];
} mem_t;

int mem_init(mem_t *memory, size_t size, const char *name);

void mem_free(mem_t *memory);

#endif // MEM_H
