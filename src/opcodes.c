#include "opcodes.h"

#include "logger.h"

static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r16_d16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_a_r16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode);

opcode_fn opcode_table[256] = {
    // Block 0
    [0x00] = op_nop, // NOP
    // Type: LD r16, imm16
    [0x01] = op_ld_r16_d16,  // LD BC,d16
    [0x11] = op_ld_r16_d16,  // LD DE,d16
    [0x21] = op_ld_r16_d16,  // LD HL,d16
    [0x31] = op_ld_r16_d16,  // LD SP,d16
    // Type: LD [r16mem], a
    [0x02] = op_ld_r16mem_a, // LD [BC],A
    [0x12] = op_ld_r16mem_a, // LD [DE],A
    [0x22] = op_ld_r16mem_a, // LD [HL+],A
    [0x32] = op_ld_r16mem_a, // LD [HL-],A
    // Type: LD a, [r16mem]
    [0x0A] = op_ld_a_r16mem, // LD A,[BC]
    [0x1A] = op_ld_a_r16mem, // LD A,[DE]
    [0x2A] = op_ld_a_r16mem, // LD A,[HL+]
    [0x3A] = op_ld_a_r16mem, // LD A,[HL-]

    // ... (initialize other opcodes as needed)
};


static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus; // Unused parameter
    (void)opcode; // Unused parameter

    LOG_DEBUG("NOP at PC=0x%04X", cpu->pc - 1);

    return 4; // NOP takes 4 cycles
}

static int op_ld_r16_d16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint8_t lo = bus_read(bus, cpu->pc);
    uint8_t hi = bus_read(bus, cpu->pc + 1);
    uint16_t immediate_value = (hi << 8) | lo;

    cpu->pc += 2;

    const char *reg_name = NULL;
    uint8_t register_code = (opcode >> 4) & 0x03;// Extract the register code from the opcode
    switch (register_code) {
        case 0b00: // BC:
            cpu->bc.reg = immediate_value;
            reg_name = "BC";
            break;
        case 0b01: // DE:
            cpu->de.reg = immediate_value;
            reg_name = "DE";
            break;
        case 0b10: // HL:
            cpu->hl.reg = immediate_value;
            reg_name = "HL";
            break;
        case 0b11: // SP:
            cpu->sp = immediate_value;
            reg_name = "SP";
            break;
    }

    LOG_DEBUG("LD %s,d16 value=0x%04X at PC=0x%04X (opcode=0x%02X)",
        reg_name, immediate_value, cpu->pc - 1, opcode);

    return 12; // LD r16,d16 takes 12 cycles
}

static int op_ld_r16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    const char *reg_name = NULL;
    uint8_t register_code = (opcode >> 4) & 0x03; // Extract the register code from the opcode
    switch (register_code) {
        case 0b00:
            bus_write(bus, cpu->bc.reg, cpu->af.hi);
            reg_name = "BC";
            break;
        case 0b01:
            bus_write(bus, cpu->de.reg, cpu->af.hi);
            reg_name = "DE";
            break;
        case 0b10:
            bus_write(bus, cpu->hl.reg++, cpu->af.hi);
            reg_name = "HL+";
            break;
        case 0b11:
            bus_write(bus, cpu->hl.reg--, cpu->af.hi);
            reg_name = "HL-";
            break;
    }

    LOG_DEBUG("LD [%s],A %s=0x%04X A=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, cpu->bc.reg, cpu->af.hi, cpu->pc - 1, opcode);

    return 8; // LD [r16mem],a takes 8 cycles
}

static int op_ld_a_r16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    const char *reg_name = NULL;
    uint16_t addr = 0;
    uint8_t register_code = (opcode >> 4) & 0x03; // Extract the register code from the opcode
    switch (register_code) {
        case 0x00:
            addr = cpu->bc.reg;
            cpu->af.hi = bus_read(bus, addr);
            reg_name = "BC";
            break;
        case 0x01:
            addr = cpu->de.reg;
            cpu->af.hi = bus_read(bus, addr);
            reg_name = "DE";
            break;
        case 0x02:
            addr = cpu->hl.reg++;
            cpu->af.hi = bus_read(bus, addr);
            reg_name = "HL+";
            break;
        case 0x03:
            addr = cpu->hl.reg--;
            cpu->af.hi = bus_read(bus, addr);
            reg_name = "HL-";
            break;
    }

    LOG_DEBUG("LD A,[%s] %s=0x%04X [%s]=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, cpu->bc.reg, reg_name, cpu->af.hi, cpu->pc - 1, opcode);

    return 8; // LD a,[r16mem] takes 8 cycles
}
