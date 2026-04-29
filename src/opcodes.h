#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

#include "bus.h"
#include "cpu.h"

typedef int (*opcode_fn)(cpu_t *cpu, bus_t *bus, uint8_t opcode);

extern opcode_fn opcode_table[256];

#endif // OPCODES_H
