#include "opcodes.h"

#include <assert.h>
#include <stdbool.h>

#include "alu.h"
#include "logger.h"

/*-------------------------------------------------------
 * Private helpers declaration
 *-------------------------------------------------------*/
static uint8_t read_imm8(cpu_t *cpu, bus_t *bus);
static uint16_t read_imm16(cpu_t *cpu, bus_t *bus);
static uint8_t read_r8(cpu_t *cpu, bus_t *bus, r8_operand_t r8_op);
static void write_r8(cpu_t *cpu, bus_t *bus, r8_operand_t r8_op, uint8_t value);
static const char *get_r8_name(r8_operand_t r8_op);
static uint16_t read_r16(cpu_t *cpu, r16_operand_t r16_op);
static void write_r16(cpu_t *cpu, r16_operand_t r16_op, uint16_t value);
static const char *get_r16_name(r16_operand_t r16_op);
static uint16_t read_r16mem(cpu_t *cpu, r16mem_operand_t r16mem_op);
static const char *get_r16mem_name(r16mem_operand_t r16mem_op);
static bool check_condition(cpu_t *cpu, cond_operand_t cond);
static const char *get_condition_name(cond_operand_t cond);

/*-------------------------------------------------------
 * Opcode declaration
 *-------------------------------------------------------*/
static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_stop(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_halt(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r16_imm16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_a_r16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_imm16mem_sp(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_inc_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_dec_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_add_hl_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_inc_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_dec_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r8_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_rlca(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_rrca(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_rla(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_rra(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_daa(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_cpl(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_scf(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ccf(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_jr_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_jr_cond_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_ld_r8_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_add_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_adc_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_sub_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_sbc_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_and_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_xor_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_or_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);
static int op_cp_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode);

opcode_fn opcode_table[256] = {
    // Block 0
    [0x00] = op_nop,          // NOP
    [0x10] = op_stop,         // STOP
    // Type: LD r16, imm16
    [0x01] = op_ld_r16_imm16, // LD BC,imm8
    [0x11] = op_ld_r16_imm16, // LD DE,imm8
    [0x21] = op_ld_r16_imm16, // LD HL,imm8
    [0x31] = op_ld_r16_imm16, // LD SP,imm8
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
    [0x09] = op_add_hl_r16,   // ADD HL,BC
    [0x19] = op_add_hl_r16,   // ADD HL,DE
    [0x29] = op_add_hl_r16,   // ADD HL,HL
    [0x39] = op_add_hl_r16,   // ADD HL,SP
    // Type: INC r8
    [0x04] = op_inc_r8,       // INC B
    [0x0C] = op_inc_r8,       // INC C
    [0x14] = op_inc_r8,       // INC D
    [0x1C] = op_inc_r8,       // INC E
    [0x24] = op_inc_r8,       // INC H
    [0x2C] = op_inc_r8,       // INC L
    [0x34] = op_inc_r8,       // INC [HL]
    [0x3C] = op_inc_r8,       // INC A
    // Type: DEC r8
    [0x05] = op_dec_r8,       // DEC B
    [0x0D] = op_dec_r8,       // DEC C
    [0x15] = op_dec_r8,       // DEC D
    [0x1D] = op_dec_r8,       // DEC E
    [0x25] = op_dec_r8,       // DEC H
    [0x2D] = op_dec_r8,       // DEC L
    [0x35] = op_dec_r8,       // DEC [HL]
    [0x3D] = op_dec_r8,       // DEC A
    // Type: LD r8, imm8
    [0x06] = op_ld_r8_imm8,   // LD B,imm8
    [0x0E] = op_ld_r8_imm8,   // LD C,imm8
    [0x16] = op_ld_r8_imm8,   // LD D,imm8
    [0x1E] = op_ld_r8_imm8,   // LD E,imm8
    [0x26] = op_ld_r8_imm8,   // LD H,imm8
    [0x2E] = op_ld_r8_imm8,   // LD L,imm8
    [0x36] = op_ld_r8_imm8,   // LD [HL],imm8
    [0x3E] = op_ld_r8_imm8,   // LD A,imm8
    // Type: Rotation
    [0x07] = op_rlca,         // RLCA
    [0x0F] = op_rrca,         // RRCA
    [0x17] = op_rla,          // RLA
    [0x1F] = op_rra,          // RRA
    // Type: DAA (Decimal Adjust Accumulator)
    [0x27] = op_daa,
    // Type: CPL (ComPLement)
    [0x2F] = op_cpl,
    // Type: SCF (Set Carry Flag)
    [0x37] = op_scf,
    // Type: CCF (Complement Carry Flag)
    [0x3F] = op_ccf,
    // Type: JR (Jump Register)
    [0x18] = op_jr_imm8,      // JR imm8
    [0x20] = op_jr_cond_imm8, // JR NZ,imm8
    [0x28] = op_jr_cond_imm8, // JR Z,imm8
    [0x30] = op_jr_cond_imm8, // JR NC,imm8
    [0x38] = op_jr_cond_imm8, // JR C,imm8

    // Block 1: 8-bit register-to-register loads
    // Type: LD r8, r8
    [0x40] = op_ld_r8_r8,     // LD B,B -> NOP
    [0x41] = op_ld_r8_r8,     // LD B,C
    [0x42] = op_ld_r8_r8,     // LD B,D
    [0x43] = op_ld_r8_r8,     // LD B,E
    [0x44] = op_ld_r8_r8,     // LD B,H
    [0x45] = op_ld_r8_r8,     // LD B,L
    [0x46] = op_ld_r8_r8,     // LD B,[HL]
    [0x47] = op_ld_r8_r8,     // LD B,A
    [0x48] = op_ld_r8_r8,     // LD C,B
    [0x49] = op_ld_r8_r8,     // LD C,C -> NOP
    [0x4A] = op_ld_r8_r8,     // LD C,D
    [0x4B] = op_ld_r8_r8,     // LD C,E
    [0x4C] = op_ld_r8_r8,     // LD C,H
    [0x4D] = op_ld_r8_r8,     // LD C,L
    [0x4E] = op_ld_r8_r8,     // LD C,[HL]
    [0x4F] = op_ld_r8_r8,     // LD C,A
    [0x50] = op_ld_r8_r8,     // LD D,B
    [0x51] = op_ld_r8_r8,     // LD D,C
    [0x52] = op_ld_r8_r8,     // LD D,D -> NOP
    [0x53] = op_ld_r8_r8,     // LD D,E
    [0x54] = op_ld_r8_r8,     // LD D,H
    [0x55] = op_ld_r8_r8,     // LD D,L
    [0x56] = op_ld_r8_r8,     // LD D,[HL]
    [0x57] = op_ld_r8_r8,     // LD D,A
    [0x58] = op_ld_r8_r8,     // LD E,B
    [0x59] = op_ld_r8_r8,     // LD E,C
    [0x5A] = op_ld_r8_r8,     // LD E,D
    [0x5B] = op_ld_r8_r8,     // LD E,E -> NOP
    [0x5C] = op_ld_r8_r8,     // LD E,H
    [0x5D] = op_ld_r8_r8,     // LD E,L
    [0x5E] = op_ld_r8_r8,     // LD E,[HL]
    [0x5F] = op_ld_r8_r8,     // LD E,A
    [0x60] = op_ld_r8_r8,     // LD H,B
    [0x61] = op_ld_r8_r8,     // LD H,C
    [0x62] = op_ld_r8_r8,     // LD H,D
    [0x63] = op_ld_r8_r8,     // LD H,E
    [0x64] = op_ld_r8_r8,     // LD H,H -> NOP
    [0x65] = op_ld_r8_r8,     // LD H,L
    [0x66] = op_ld_r8_r8,     // LD H,[HL]
    [0x67] = op_ld_r8_r8,     // LD H,A
    [0x68] = op_ld_r8_r8,     // LD L,B
    [0x69] = op_ld_r8_r8,     // LD L,C
    [0x6A] = op_ld_r8_r8,     // LD L,D
    [0x6B] = op_ld_r8_r8,     // LD L,E
    [0x6C] = op_ld_r8_r8,     // LD L,H
    [0x6D] = op_ld_r8_r8,     // LD L,L -> NOP
    [0x6E] = op_ld_r8_r8,     // LD L,[HL]
    [0x6F] = op_ld_r8_r8,     // LD L,A
    [0x70] = op_ld_r8_r8,     // LD [HL],B
    [0x71] = op_ld_r8_r8,     // LD [HL],C
    [0x72] = op_ld_r8_r8,     // LD [HL],D
    [0x73] = op_ld_r8_r8,     // LD [HL],E
    [0x74] = op_ld_r8_r8,     // LD [HL],H
    [0x75] = op_ld_r8_r8,     // LD [HL],L
    [0x76] = op_halt,         // LD [HL],[HL] -> HALT
    [0x77] = op_ld_r8_r8,     // LD [HL],A
    [0x78] = op_ld_r8_r8,     // LD A,B
    [0x79] = op_ld_r8_r8,     // LD A,C
    [0x7A] = op_ld_r8_r8,     // LD A,D
    [0x7B] = op_ld_r8_r8,     // LD A,E
    [0x7C] = op_ld_r8_r8,     // LD A,H
    [0x7D] = op_ld_r8_r8,     // LD A,L
    [0x7E] = op_ld_r8_r8,     // LD A,[HL]
    [0x7F] = op_ld_r8_r8,     // LD A,A -> NOP

    // Block 2: 8-bit arithmetic
    // Type: ADD a, r8
    [0x80] = op_add_a_r8,     // ADD A,B
    [0x81] = op_add_a_r8,     // ADD A,C
    [0x82] = op_add_a_r8,     // ADD A,D
    [0x83] = op_add_a_r8,     // ADD A,E
    [0x84] = op_add_a_r8,     // ADD A,H
    [0x85] = op_add_a_r8,     // ADD A,L
    [0x86] = op_add_a_r8,     // ADD A,[HL]
    [0x87] = op_add_a_r8,     // ADD A,A
    //Type: ADC a, r8
    [0x88] = op_adc_a_r8,     // ADC a,B
    [0x89] = op_adc_a_r8,     // ADC a,C
    [0x8A] = op_adc_a_r8,     // ADC a,D
    [0x8B] = op_adc_a_r8,     // ADC a,E
    [0x8C] = op_adc_a_r8,     // ADC a,H
    [0x8D] = op_adc_a_r8,     // ADC a,L
    [0x8E] = op_adc_a_r8,     // ADC a,[HL]
    [0x8F] = op_adc_a_r8,     // ADC a,A
    //Type: SUB a, r8
    [0x90] = op_sub_a_r8,     // SUB A,B
    [0x91] = op_sub_a_r8,     // SUB A,C
    [0x92] = op_sub_a_r8,     // SUB A,D
    [0x93] = op_sub_a_r8,     // SUB A,E
    [0x94] = op_sub_a_r8,     // SUB A,H
    [0x95] = op_sub_a_r8,     // SUB A,L
    [0x96] = op_sub_a_r8,     // SUB A,[HL]
    [0x97] = op_sub_a_r8,     // SUB A,A
    //Type: SBC a, r8
    [0x98] = op_sbc_a_r8,     // SBC a,B
    [0x99] = op_sbc_a_r8,     // SBC a,C
    [0x9A] = op_sbc_a_r8,     // SBC a,D
    [0x9B] = op_sbc_a_r8,     // SBC a,E
    [0x9C] = op_sbc_a_r8,     // SBC a,H
    [0x9D] = op_sbc_a_r8,     // SBC a,L
    [0x9E] = op_sbc_a_r8,     // SBC a,[HL]
    [0x9F] = op_sbc_a_r8,     // SBC a,A
    // Type: AND A, r8
    [0xA0] = op_and_a_r8,     // AND a,B
    [0xA1] = op_and_a_r8,     // AND a,C
    [0xA2] = op_and_a_r8,     // AND a,D
    [0xA3] = op_and_a_r8,     // AND a,E
    [0xA4] = op_and_a_r8,     // AND a,H
    [0xA5] = op_and_a_r8,     // AND a,L
    [0xA6] = op_and_a_r8,     // AND a,[HL]
    [0xA7] = op_and_a_r8,     // AND a,A
    //Type: XOR a, r8
    [0xA8] = op_xor_a_r8,     // XOR a,B
    [0xA9] = op_xor_a_r8,     // XOR a,C
    [0xAA] = op_xor_a_r8,     // XOR a,D
    [0xAB] = op_xor_a_r8,     // XOR a,E
    [0xAC] = op_xor_a_r8,     // XOR a,H
    [0xAD] = op_xor_a_r8,     // XOR a,L
    [0xAE] = op_xor_a_r8,     // XOR a,[HL]
    [0xAF] = op_xor_a_r8,     // XOR a,A
    // Type: OR A, r8
    [0xB0] = op_or_a_r8,      // OR a,B
    [0xB1] = op_or_a_r8,      // OR a,C
    [0xB2] = op_or_a_r8,      // OR a,D
    [0xB3] = op_or_a_r8,      // OR a,E
    [0xB4] = op_or_a_r8,      // OR a,H
    [0xB5] = op_or_a_r8,      // OR a,L
    [0xB6] = op_or_a_r8,      // OR a,[HL]
    [0xB7] = op_or_a_r8,      // OR a,A
    //Type: CP a, r8 (ComPare)
    [0xB8] = op_cp_a_r8,      // CP a,B
    [0xB9] = op_cp_a_r8,      // CP a,C
    [0xBA] = op_cp_a_r8,      // CP a,D
    [0xBB] = op_cp_a_r8,      // CP a,E
    [0xBC] = op_cp_a_r8,      // CP a,H
    [0xBD] = op_cp_a_r8,      // CP a,L
    [0xBE] = op_cp_a_r8,      // CP a,[HL]
    [0xBF] = op_cp_a_r8,      // CP a,A

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

static int op_stop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;

    // TODO: Understand how STOP actually works

    read_imm8(cpu, bus);

    LOG_DEBUG("STOP at PC=0x%04X", cpu->pc - 1);

    return 4; // STOP takes 4 cycles
}

static int op_halt(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;
    (void)bus;

    // TODO: Understand how HALT actually works

    LOG_DEBUG("HALT at PC=0x%04X", cpu->pc - 1);

    return 4; // HALT takes 4 cycles
}

static int op_ld_r16_imm16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t immediate_value = read_imm16(cpu, bus);

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, immediate_value);

    LOG_DEBUG("LD %s,imm16 value=0x%04X at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), immediate_value, instr_pc, opcode);

    return 12; // LD r16,imm16 takes 12 cycles
}

static int op_ld_r16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r16mem_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16mem(cpu, register_code);
    const char *reg_name = get_r16mem_name(register_code);

    bus_write(bus, reg_value, cpu->af.hi);

    LOG_DEBUG("LD [%s],A %s=0x%04X A=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, cpu->bc.reg, cpu->af.hi, instr_pc, opcode);

    return 8; // LD [r16mem],a takes 8 cycles
}

static int op_ld_a_r16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r16mem_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16mem(cpu, register_code);
    const char *reg_name = get_r16mem_name(register_code);

    cpu->af.hi = bus_read(bus, reg_value);

    LOG_DEBUG("LD A,[%s] %s=0x%04X [%s]=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, cpu->bc.reg, reg_name, cpu->af.hi, instr_pc, opcode);

    return 8; // LD a,[r16mem] takes 8 cycles
}

static int op_ld_imm16mem_sp(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t address = read_imm16(cpu, bus);

    bus_write(bus, address, cpu->sp & 0xFF);
    bus_write(bus, address + 1, cpu->sp >> 8);

    LOG_DEBUG("LD [0x%04X],SP SP=0x%04X at PC=0x%04X (opcode=0x%02X)",
        address, cpu->sp, instr_pc, opcode);

    return 20; // LD [imm16],sp takes 20 cycles
}

static int op_inc_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, read_r16(cpu, register_code) + 1);

    LOG_DEBUG("INC %s at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), instr_pc, opcode);

    return 8; // INC r16 takes 8 cycles
}

static int op_dec_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, read_r16(cpu, register_code) - 1);

    LOG_DEBUG("DEC %s at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), instr_pc, opcode);

    return 8; // DEC r16 takes 8 cycles
}

static int op_add_hl_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

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
        get_r16_name(register_code), cpu->hl.reg, instr_pc, opcode);

    return 8; // ADD HL,r16 takes 8 cycles
}

static int op_inc_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = (opcode >> 3) & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);

    alu8_result_t result = alu_inc8(reg_value);

    write_r8(cpu, bus, register_code, result.value);

    flag_clear(cpu, FLAG_N);
    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);

    LOG_DEBUG("INC %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 12 : 4; // INC r8 takes 4 cycles for normal r8 register and 12 for [HL]
}

static int op_dec_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = (opcode >> 3) & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);

    alu8_result_t result = alu_dec8(reg_value);

    write_r8(cpu, bus, register_code, result.value);

    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);

    LOG_DEBUG("DEC %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 12 : 4; // DEC r8 takes 4 cycles for normal r8 register and 12 for [HL]
}

static int op_ld_r8_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);

    r8_operand_t register_code = (opcode >> 3) & 0b111; // Extract the register code from the opcode

    write_r8(cpu, bus, register_code, immediate_value);

    LOG_DEBUG("LD %s,imm8 value=0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), immediate_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 12 : 4; // LD r8,imm8 takes 4 cycles for normal r8 register and 12 for [HL]
}

static int op_rlca(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a >> 7) == 1;

    cpu->af.hi = (old_a << 1) | (old_a >> 7);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RLCA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RLCA takes 4 cycles
}

static int op_rrca(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a & 0x01) == 1;

    cpu->af.hi = (old_a >> 1) | ((old_a & 0x01) << 7);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RRCA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RRCA takes 4 cycles
}

static int op_rla(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a >> 7) == 1;

    cpu->af.hi = (old_a << 1) | (flag_get(cpu, FLAG_C) ? 1 : 0);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RLA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RLA takes 4 cycles
}

static int op_rra(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a & 0x01) == 1;

    cpu->af.hi = (flag_get(cpu, FLAG_C) ? 0x80 : 0x00) | (old_a >> 1);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RRA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RRA takes 4 cycles
}

static int op_daa(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    bool subtraction_flag = flag_get(cpu, FLAG_N);
    bool half_carry_flag = flag_get(cpu, FLAG_H);
    bool carry_flag = flag_get(cpu, FLAG_C);

    uint8_t adjustment = 0;

    if (subtraction_flag) {
        if (half_carry_flag) adjustment += 0x06;
        if (carry_flag) adjustment += 0x60;
        cpu->af.hi -= adjustment;
    } else {
        if (half_carry_flag || (cpu->af.hi & 0xF) > 0x9) {
            adjustment += 0x06;
        }
        if (carry_flag || cpu->af.hi > 0x99) {
            flag_set(cpu, FLAG_C);
            adjustment += 0x60;
        }
        cpu->af.hi += adjustment;
    }

    flag_clear(cpu, FLAG_H);
    if (cpu->af.hi == 0) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);

    LOG_DEBUG("DAA A=0x%02X at PC=0x%04X (opcode=0x%02X)",
        cpu->af.hi, instr_pc, opcode);

    return 4; // DAA takes 4 cycles
}

static int op_cpl(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    cpu->af.hi = ~cpu->af.hi;

    flag_set(cpu, FLAG_N);
    flag_set(cpu, FLAG_H);

    LOG_DEBUG("CPL A=0x%02X at PC=0x%04X (opcode=0x%02X)",
        cpu->af.hi, instr_pc, opcode);

    return 4; // CPL takes 4 cycles
}

static int op_scf(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_set(cpu, FLAG_C);

    LOG_DEBUG("SCF at PC=0x%04X (opcode=0x%02X)", instr_pc, opcode);

    return 4; // SCF takes 4 cycles
}

static int op_ccf(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);

    if (flag_get(cpu, FLAG_C)) flag_clear(cpu, FLAG_C); else flag_set(cpu, FLAG_C);

    LOG_DEBUG("CFF at PC=0x%04X (opcode=0x%02X)", instr_pc, opcode);

    return 4; // CCF takes 4 cycles
}

static int op_jr_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    int8_t offset = (int8_t)read_imm8(cpu, bus);

    cpu->pc += offset;

    LOG_DEBUG("JR imm8 offset=%d at PC=0x%04X (opcode=0x%02X)",
        offset, instr_pc, opcode);

    return 12; // JR imm8 takes 12 cycles
}

static int op_jr_cond_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    cond_operand_t cond_op = (opcode >> 3) & 0b11; // Extract condition code from opcode
    bool condition = check_condition(cpu, cond_op);

    int8_t offset = (int8_t)read_imm8(cpu, bus);

    if (condition) {
        cpu->pc += offset;
    }

    LOG_DEBUG("JR %s offset=%d (0x%02X) at PC=0x%04X (opcode=0x%02X)",
        get_condition_name(cond_op), offset, (uint8_t)offset, instr_pc, opcode);

    return condition ? 12 : 8;
}

static int op_ld_r8_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t dest_reg_code = (opcode >> 3) & 0b111;
    r8_operand_t source_reg_code = opcode & 0b111;

    write_r8(cpu, bus, dest_reg_code, read_r8(cpu, bus, source_reg_code));

    LOG_DEBUG("LD %s,%s at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(dest_reg_code), get_r8_name(source_reg_code), instr_pc, opcode);

    return (dest_reg_code == OP_MEM_HL || source_reg_code == OP_MEM_HL) ? 8 : 4;
}

static int op_add_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t alu_result = alu_add8(a, reg_value, 0);
    cpu->af.hi = alu_result.value;

    // N cleared, Z, H and C set according to result
    flag_clear(cpu, FLAG_N);
    if (alu_result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (alu_result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (alu_result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADD A,%s: 0x%02X + 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, alu_result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // ADD A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_adc_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 1 : 0;

    alu8_result_t alu_result = alu_add8(a, reg_value, carry_in);
    cpu->af.hi = alu_result.value;

    // N cleared, Z, H and C set according to result
    flag_clear(cpu, FLAG_N);
    if (alu_result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (alu_result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (alu_result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADC A,%s: 0x%02X + 0x%02X + %d = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, carry_in, alu_result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // ADC A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_sub_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_sub8(a, reg_value, 0);
    cpu->af.hi = result.value;

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SUB A,%s: 0x%02X - 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // SUB A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_sbc_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 1 : 0;

    alu8_result_t result = alu_sub8(a, reg_value, carry_in);
    cpu->af.hi = result.value;

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)        flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SBC A,%s: 0x%02X - 0x%02X - %d = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, carry_in, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // SBC A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_and_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_and8(a, reg_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_set(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("AND A,%s: 0x%02X & 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // AND A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_xor_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_xor8(a, reg_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("XOR A,%s: 0x%02X ^ 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // XOR A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_or_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_or8(a, reg_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("OR A,%s: 0x%02X | 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // OR A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_cp_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_sub8(a, reg_value, 0);

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("CP A,%s: 0x%02X - 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // CP A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

/*-------------------------------------------------------
 * Private helpers definition
 *-------------------------------------------------------*/

static uint8_t read_imm8(cpu_t *cpu, bus_t *bus) {
    uint8_t immediate_value = bus_read(bus, cpu->pc);
    cpu->pc += 1;
    return immediate_value;
}

static uint16_t read_imm16(cpu_t *cpu, bus_t *bus) {
    uint8_t lo = bus_read(bus, cpu->pc);
    uint8_t hi = bus_read(bus, cpu->pc + 1);
    cpu->pc += 2;
    return (hi << 8) | lo;
}

static uint8_t read_r8(cpu_t *cpu, bus_t *bus, r8_operand_t r8_op) {
    switch (r8_op) {
        case OP_REG_B: return cpu->bc.hi;
        case OP_REG_C: return cpu->bc.lo;
        case OP_REG_D: return cpu->de.hi;
        case OP_REG_E: return cpu->de.lo;
        case OP_REG_H: return cpu->hl.hi;
        case OP_REG_L: return cpu->hl.lo;
        case OP_MEM_HL: return bus_read(bus, cpu->hl.reg); // [HL]
        case OP_REG_A: return cpu->af.hi;
        default: assert(0 && "Invalid r8 operand");
    }
}

static void write_r8(cpu_t *cpu, bus_t *bus, r8_operand_t r8_op, uint8_t value) {
    switch (r8_op) {
        case OP_REG_B: cpu->bc.hi = value; break;
        case OP_REG_C: cpu->bc.lo = value; break;
        case OP_REG_D: cpu->de.hi = value; break;
        case OP_REG_E: cpu->de.lo = value; break;
        case OP_REG_H: cpu->hl.hi = value; break;
        case OP_REG_L: cpu->hl.lo = value; break;
        case OP_MEM_HL: bus_write(bus, cpu->hl.reg, value); break; // [HL]
        case OP_REG_A: cpu->af.hi = value; break;
        default: assert(0 && "Invalid r8 operand");
    }
}

static const char *get_r8_name(r8_operand_t r8_op) {
    static const char *names[] = {
        "B","C","D","E","H","L","[HL]","A"
    };
    if (r8_op > OP_REG_A) return "??";
    return names[r8_op];
}

static uint16_t read_r16(cpu_t *cpu, r16_operand_t r16_op) {
    switch (r16_op) {
        case OP_REG_BC: return cpu->bc.reg;
        case OP_REG_DE: return cpu->de.reg;
        case OP_REG_HL: return cpu->hl.reg;
        case OP_REG_SP: return cpu->sp;
        default: assert(0 && "Invalid r16 operand");
    }
}

static void write_r16(cpu_t *cpu, r16_operand_t r16_op, uint16_t value) {
    switch (r16_op) {
        case OP_REG_BC: cpu->bc.reg = value; break;
        case OP_REG_DE: cpu->de.reg = value; break;
        case OP_REG_HL: cpu->hl.reg = value; break;
        case OP_REG_SP: cpu->sp = value; break;
        default: assert(0 && "Invalid r16 operand");
    }
}

static const char *get_r16_name(r16_operand_t r16_op) {
    static const char *names[] = {
        "BC","DE","HL","SP"
    };
    if (r16_op > OP_REG_SP) return "??";
    return names[r16_op];
}

static uint16_t read_r16mem(cpu_t *cpu, r16mem_operand_t r16mem_op) {
    switch (r16mem_op) {
        case OP_REG_BC_MEM: return cpu->bc.reg;
        case OP_REG_DE_MEM: return cpu->de.reg;
        case OP_REG_HLI_MEM: return cpu->hl.reg++;
        case OP_REG_HLD_MEM: return cpu->hl.reg--;
        default: assert(0 && "Invalid r16mem operand");
    }
}

static const char *get_r16mem_name(r16mem_operand_t r16mem_op) {
    static const char *names[] = {
        "BC","DE","HL+","HL-"
    };
    if (r16mem_op > OP_REG_HLD_MEM) return "??";
    return names[r16mem_op];
}

static bool check_condition(cpu_t *cpu, cond_operand_t cond) {
    switch (cond) {
        case OP_NZ: return !flag_get(cpu, FLAG_Z);
        case OP_Z: return  flag_get(cpu, FLAG_Z);
        case OP_NC: return !flag_get(cpu, FLAG_C);
        case OP_C: return  flag_get(cpu, FLAG_C);
        default: assert(0 && "Invalid condition operand");
    }
}

static const char *get_condition_name(cond_operand_t cond) {
    static const char *names[] = {
        "NZ","Z","NC","C"
    };
    if (cond > OP_C) return "??";
    return names[cond];
}
