#include "opcodes.h"

#include "logger.h"

static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r16_d16(cpu_t *cpu, bus_t *bus, uint8_t opcode);

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


static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus; // Unused parameter
    (void)opcode; // Unused parameter

    LOG_DEBUG("NOP at PC=0x%04X", cpu->pc);

    return 4; // NOP takes 4 cycles
}

static int op_ld_r16_d16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {

    uint16_t instr_pc = cpu->pc - 1;

    uint8_t lo = bus_read(bus, cpu->pc);
    uint8_t hi = bus_read(bus, cpu->pc + 1);
    uint16_t immediate_value = (hi << 8) | lo;

    cpu->pc += 2;

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


    LOG_DEBUG("LD r16,d16 register=%u value=0x%04X at PC=0x%04X (opcode=0x%02X)",
        register_code, immediate_value, instr_pc, opcode);

    return 12; // LD r16,d16 takes 12 cycles
}
