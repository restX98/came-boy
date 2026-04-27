#include "opcodes.h"
#include "logger.h"

opcode_fn opcode_table[256] = {
    [0x00] = op_nop, // NOP
};


int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    LOG_DEBUG("NOP at PC=0x%04X", cpu->pc);

    (void)bus; // Unused parameter
    (void)opcode; // Unused parameter
    cpu->pc += 1;

    return 4; // NOP takes 4 cycles
}
