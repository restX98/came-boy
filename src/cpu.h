#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "bus.h"

#define FLAG_Z (1 << 7) // 10000000
#define FLAG_N (1 << 6) // 01000000
#define FLAG_H (1 << 5) // 00100000
#define FLAG_C (1 << 4) // 00010000

typedef union {
    uint16_t reg;
    struct {
        uint8_t lo;
        uint8_t hi;
    };
} reg16_t;

typedef struct {
    reg16_t af;
    reg16_t bc;
    reg16_t de;
    reg16_t hl;
    uint16_t sp;
    uint16_t pc;

    struct {
        bool enabled;   // Interrupt Master Enable
        bool scheduled; // For EI's one-instruction delay
    } ime;
} cpu_t;

void cpu_init(cpu_t *cpu);

int cpu_step(cpu_t *cpu, bus_t *bus);

static inline void flag_set(cpu_t *cpu, uint8_t flag) {
    cpu->af.lo |= flag;
}

static inline void flag_clear(cpu_t *cpu, uint8_t flag) {
    cpu->af.lo &= ~flag;
}

static inline bool flag_get(cpu_t *cpu, uint8_t flag) {
    return cpu->af.lo & flag;
}

#endif // CPU_H
