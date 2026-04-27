#include "opcodes.h"
#include "logger.h"

opcode_fn opcode_table[256] = {
    // Block 0
    [0x00] = op_nop, // NOP
    // Type: LD r16, imm16
    [0x01] = op_ld_r16_d16, // LD BC,d16
    [0x11] = op_ld_r16_d16, // LD DE,d16
    [0x21] = op_ld_r16_d16, // LD HL,d16
    [0x31] = op_ld_r16_d16, // LD SP,d16
    // ... (initialize other opcodes as needed)
};


int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus; // Unused parameter
    (void)opcode; // Unused parameter

    LOG_DEBUG("NOP at PC=0x%04X", cpu->pc);

    cpu->pc += 1;

    return 4; // NOP takes 4 cycles
}

int op_ld_r16_d16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t immediate_value = (bus_read(bus, cpu->pc + 2) << 8) | bus_read(bus, cpu->pc + 1); // Read 16-bit immediate value

    // Extract the register code from the opcode
    uint8_t register_code = (opcode >> 4) & 0x03;
    switch (register_code) {
        case 0x00: // BC:
            cpu->bc.reg = immediate_value;
            break;
        case 0x01: // DE:
            cpu->de.reg = immediate_value;
            break;
        case 0x02: // HL:
            cpu->hl.reg = immediate_value;
            break;
        case 0x03: // SP:
            cpu->sp = immediate_value;
            break;
    }


    LOG_DEBUG("LD r16,d16 (%u, %d) at PC=0x%04X (opcode=0x%02X)",
        register_code, immediate_value, cpu->pc, opcode);

    cpu->pc += 3;

    return 12; // LD r16,d16 takes 12 cycles
}
