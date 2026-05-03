#include "opcodes.h"

#include <stdbool.h>

#include "logger.h"

/*-------------------------------------------------------
 * Private helpers declaration
 *-------------------------------------------------------*/
static uint16_t read_imm16(cpu_t *cpu, bus_t *bus);
static uint8_t read_r8(cpu_t *cpu, bus_t *bus, uint8_t register_code);
static void write_r8(cpu_t *cpu, bus_t *bus, uint8_t register_code, uint8_t value);
static const char *get_r8_name(uint8_t register_code);
static uint16_t read_r16(cpu_t *cpu, uint8_t register_code);
static void write_r16(cpu_t *cpu, uint8_t register_code, uint16_t value);
static const char *get_r16_name(uint8_t register_code);
static uint16_t read_r16mem(cpu_t *cpu, uint8_t register_code);
static const char *get_r16mem_name(uint8_t register_code);

/*-------------------------------------------------------
 * Opcode declaration
 *-------------------------------------------------------*/
static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r16_imm16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_a_r16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_imm16mem_sp(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_inc_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_dec_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_add_hl_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_inc_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);

opcode_fn opcode_table[256] = {
    // Block 0
    [0x00] = op_nop, // NOP
    // Type: LD r16, imm16
    [0x01] = op_ld_r16_imm16, // LD BC,d16
    [0x11] = op_ld_r16_imm16, // LD DE,d16
    [0x21] = op_ld_r16_imm16, // LD HL,d16
    [0x31] = op_ld_r16_imm16, // LD SP,d16
    // Type: LD [r16mem], a
    [0x02] = op_ld_r16mem_a,  // LD [BC],A
    [0x12] = op_ld_r16mem_a,  // LD [DE],A
    [0x22] = op_ld_r16mem_a,  // LD [HL+],A
    [0x32] = op_ld_r16mem_a,  // LD [HL-],A
    // Type: LD a, [r16mem]
    [0x0A] = op_ld_a_r16mem,  // LD A,[BC]
    [0x1A] = op_ld_a_r16mem,  // LD A,[DE]
    [0x2A] = op_ld_a_r16mem,  // LD A,[HL+]
    [0x3A] = op_ld_a_r16mem,  // LD A,[HL-]
    // Type: LD [imm16], sp
    [0x08] = op_ld_imm16mem_sp,
    // Type: INC r16
    [0x03] = op_inc_r16,      // INC BC
    [0x13] = op_inc_r16,      // INC DE
    [0x23] = op_inc_r16,      // INC HL
    [0x33] = op_inc_r16,      // INC SP
    // Type: DEC r16
    [0x0B] = op_dec_r16,      // DEC BC
    [0x1B] = op_dec_r16,      // DEC DE
    [0x2B] = op_dec_r16,      // DEC HL
    [0x3B] = op_dec_r16,      // DEC SP
    // Type: ADD hl, r16
    [0x09] = op_add_hl_r16,   // ADD HL, BC
    [0x19] = op_add_hl_r16,   // ADD HL, DE
    [0x29] = op_add_hl_r16,   // ADD HL, HL
    [0x39] = op_add_hl_r16,   // ADD HL, SP
    // Type: INC r8
    [0x04] = op_inc_r8,       // INC B
    [0x0C] = op_inc_r8,       // INC C
    [0x14] = op_inc_r8,       // INC D
    [0x1C] = op_inc_r8,       // INC E
    [0x24] = op_inc_r8,       // INC H
    [0x2C] = op_inc_r8,       // INC L
    [0x34] = op_inc_r8,       // INC [HL]
    [0x3C] = op_inc_r8,       // INC A
    // ... (initialize other opcodes as needed)
};

/*-------------------------------------------------------
 * Opcode definitions
 *-------------------------------------------------------*/

static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus; (void)opcode;

    LOG_DEBUG("NOP at PC=0x%04X", cpu->pc - 1);

    return 4; // NOP takes 4 cycles
}

static int op_ld_r16_imm16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t immediate_value = read_imm16(cpu, bus);

    uint8_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, immediate_value);

    LOG_DEBUG("LD %s,imm16 value=0x%04X at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), immediate_value, cpu->pc - 1, opcode);

    return 12; // LD r16,d16 takes 12 cycles
}

static int op_ld_r16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint8_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16mem(cpu, register_code);
    const char *reg_name = get_r16mem_name(register_code);

    bus_write(bus, reg_value, cpu->af.hi);

    LOG_DEBUG("LD [%s],A %s=0x%04X A=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, cpu->bc.reg, cpu->af.hi, cpu->pc - 1, opcode);

    return 8; // LD [r16mem],a takes 8 cycles
}

static int op_ld_a_r16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint8_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16mem(cpu, register_code);
    const char *reg_name = get_r16mem_name(register_code);

    cpu->af.hi = bus_read(bus, reg_value);

    LOG_DEBUG("LD A,[%s] %s=0x%04X [%s]=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, cpu->bc.reg, reg_name, cpu->af.hi, cpu->pc - 1, opcode);

    return 8; // LD a,[r16mem] takes 8 cycles
}

static int op_ld_imm16mem_sp(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t address = read_imm16(cpu, bus);

    bus_write(bus, address, cpu->sp & 0xFF);
    bus_write(bus, address + 1, cpu->sp >> 8);

    LOG_DEBUG("LD [0x%04X],SP SP=0x%04X at PC=0x%04X (opcode=0x%02X)",
        address, cpu->sp, cpu->pc - 1, opcode);

    return 20; // LD [imm16],sp takes 20 cycles
}

static int op_inc_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;

    uint8_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, read_r16(cpu, register_code) + 1);

    LOG_DEBUG("INC %s at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), cpu->pc - 1, opcode);

    return 8; // INC r16 takes 8 cycles
}

static int op_dec_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;

    uint8_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, read_r16(cpu, register_code) - 1);

    LOG_DEBUG("INC %s at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), cpu->pc - 1, opcode);

    return 8; // INC r16 takes 8 cycles
}

static int op_add_hl_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;

    uint8_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16(cpu, register_code);
    uint16_t old_hl = cpu->hl.reg;

    // H: overflow from bit 11
    bool half_carry = ((old_hl & 0x0FFF) + (reg_value & 0x0FFF)) > 0x0FFF;

    // C: overflow from bit 15
    bool carry = ((uint32_t)old_hl + reg_value) > 0xFFFF;

    cpu->hl.reg = old_hl + reg_value;

    // N cleared, H and C set according to result, Z unaffected
    flag_clear(cpu, FLAG_N);
    if (half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADD HL,%s HL=0x%04X at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), cpu->hl.reg, cpu->pc - 1, opcode);

    return 8; // ADD HL,r16 takes 8 cycles
}

static int op_inc_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint8_t register_code = (opcode >> 3) & 0b111; // Extract the register code from the opcode

    uint8_t old_val = read_r8(cpu, bus, register_code);
    uint8_t new_val = old_val + 1;

    bool half_carry = (old_val & 0x0F) == 0x0F;
    bool zero = new_val == 0;

    write_r8(cpu, bus, register_code, new_val);

    flag_clear(cpu, FLAG_N);
    if (zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);

    LOG_DEBUG("INC %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), old_val, new_val, cpu->pc - 1, opcode);

    return (register_code == 0b110) ? 12 : 4; // INC r8 takes 4 cycles for normal r8 register and 12 for [HL]
}

/*-------------------------------------------------------
 * Private helpers definition
 *-------------------------------------------------------*/

static uint16_t read_imm16(cpu_t *cpu, bus_t *bus) {
    uint8_t lo = bus_read(bus, cpu->pc);
    uint8_t hi = bus_read(bus, cpu->pc + 1);
    cpu->pc += 2;
    return (hi << 8) | lo;
}

static uint8_t read_r8(cpu_t *cpu, bus_t *bus, uint8_t register_code) {
    switch (register_code) {
        case 0b000: return cpu->bc.hi;
        case 0b001: return cpu->bc.lo;
        case 0b010: return cpu->de.hi;
        case 0b011: return cpu->de.lo;
        case 0b100: return cpu->hl.hi;
        case 0b101: return cpu->hl.lo;
        case 0b110: return bus_read(bus, cpu->hl.reg); // [HL]
        case 0b111: return cpu->af.hi;
        default:    return 0;
    }
}

static void write_r8(cpu_t *cpu, bus_t *bus, uint8_t register_code, uint8_t value) {
    switch (register_code) {
        case 0b000: cpu->bc.hi = value; break;
        case 0b001: cpu->bc.lo = value; break;
        case 0b010: cpu->de.hi = value; break;
        case 0b011: cpu->de.lo = value; break;
        case 0b100: cpu->hl.hi = value; break;
        case 0b101: cpu->hl.lo = value; break;
        case 0b110: bus_write(bus, cpu->hl.reg, value); break; // [HL]
        case 0b111: cpu->af.hi = value; break;
    }
}

static const char *get_r8_name(uint8_t register_code) {
    switch (register_code) {
        case 0b000: return "B";
        case 0b001: return "C";
        case 0b010: return "D";
        case 0b011: return "E";
        case 0b100: return "H";
        case 0b101: return "L";
        case 0b110: return "[HL]";
        case 0b111: return "A";
        default: return "??";
    }
}

static uint16_t read_r16(cpu_t *cpu, uint8_t register_code) {
    switch (register_code) {
        case 0b00: return cpu->bc.reg;
        case 0b01: return cpu->de.reg;
        case 0b10: return cpu->hl.reg;
        case 0b11: return cpu->sp;
        default:   return 0;
    }
}

static void write_r16(cpu_t *cpu, uint8_t register_code, uint16_t value) {
    switch (register_code) {
        case 0b00: cpu->bc.reg = value; break;
        case 0b01: cpu->de.reg = value; break;
        case 0b10: cpu->hl.reg = value; break;
        case 0b11: cpu->sp = value; break;
    }
}

static const char *get_r16_name(uint8_t register_code) {
    switch (register_code) {
        case 0b00: return "BC";
        case 0b01: return "DE";
        case 0b10: return "HL";
        case 0b11: return "SP";
        default: return "??";
    }
}

static uint16_t read_r16mem(cpu_t *cpu, uint8_t register_code) {
    switch (register_code) {
        case 0b00: return cpu->bc.reg;
        case 0b01: return cpu->de.reg;
        case 0b10: return cpu->hl.reg++;
        case 0b11: return cpu->hl.reg--;
        default: return -1;
    }
}

static const char *get_r16mem_name(uint8_t register_code) {
    switch (register_code) {
        case 0b00: return "BC";
        case 0b01: return "DE";
        case 0b10: return "HL+";
        case 0b11: return "HL-";
        default: return "??";
    }
}
