#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "bus.h"

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
} cpu_t;

void cpu_init(cpu_t *cpu);

int cpu_step(cpu_t *cpu, bus_t *bus);

#endif // CPU_H
