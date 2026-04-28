#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

#include "bus.h"
#include "cpu.h"

typedef int (*opcode_fn)(cpu_t *cpu, bus_t *bus, uint8_t opcode);

extern opcode_fn opcode_table[256];

// Opcode function declarations
int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode);

int op_ld_r16_d16(cpu_t *cpu, bus_t *bus, uint8_t opcode);

#endif // OPCODES_H
