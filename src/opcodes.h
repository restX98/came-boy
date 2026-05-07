#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

#include "bus.h"
#include "cpu.h"

typedef enum {
    OP_REG_B = 0,
    OP_REG_C,
    OP_REG_D,
    OP_REG_E,
    OP_REG_H,
    OP_REG_L,
    OP_MEM_HL,
    OP_REG_A
} r8_operand_t;

typedef enum {
    OP_REG_BC = 0,
    OP_REG_DE,
    OP_REG_HL,
    OP_REG_SP,
} r16_operand_t;

typedef enum {
    OP_REG_BC_MEM = 0,
    OP_REG_DE_MEM,
    OP_REG_HLI_MEM,
    OP_REG_HLD_MEM,
} r16mem_operand_t;

typedef enum {
    OP_NZ = 0,
    OP_Z,
    OP_NC,
    OP_C,
} cond_operand_t;

typedef enum {
    OP_REG_BC_STK = 0,
    OP_REG_DE_STK,
    OP_REG_HL_STK,
    OP_REG_AF_STK,
} r16stk_operand_t;

typedef int (*opcode_fn)(cpu_t *cpu, bus_t *bus, uint8_t opcode);

extern opcode_fn opcode_table[256];

#endif // OPCODES_H
