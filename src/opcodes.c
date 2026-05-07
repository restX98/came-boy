#include "opcodes.h"

#include <assert.h>
#include <stdbool.h>

#include "alu.h"
#include "logger.h"

#include "opcodes/opcode_helpers.inl"

#include "opcodes/opcodes_alu8.inl"
#include "opcodes/opcodes_alu16.inl"
#include "opcodes/opcodes_bit_shift.inl"
#include "opcodes/opcodes_bitwise.inl"
#include "opcodes/opcodes_carry_flag.inl"
#include "opcodes/opcodes_interrupt.inl"
#include "opcodes/opcodes_jump.inl"
#include "opcodes/opcodes_load.inl"
#include "opcodes/opcodes_misc.inl"

opcode_fn opcode_table[256] = {
    // Block 0
    [0x00] = op_nop,          // NOP
    [0x10] = op_stop,         // STOP
    // Type: LD r16, imm16
    [0x01] = op_ld_r16_imm16, // LD BC,imm16
    [0x11] = op_ld_r16_imm16, // LD DE,imm16
    [0x21] = op_ld_r16_imm16, // LD HL,imm16
    [0x31] = op_ld_r16_imm16, // LD SP,imm16
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

    // Block 3:
    // Type: ADD a, imm8
    [0xC6] = op_add_a_imm8,
    // Type: ADC a, imm8
    [0xCE] = op_adc_a_imm8,
    // Type: SUB a, imm8
    [0xD6] = op_sub_a_imm8,
    // Type: SBC a, imm8
    [0xDE] = op_sbc_a_imm8,
    // Type: AND a, imm8
    [0xE6] = op_and_a_imm8,
    // Type: XOR a, imm8
    [0xEE] = op_xor_a_imm8,
    // Type: OR a, imm8
    [0xF6] = op_or_a_imm8,
    // Type: CP a, imm8
    [0xFE] = op_cp_a_imm8,
    // Type: RET cond
    [0xC0] = op_ret_cond,     // RET NZ
    [0xC8] = op_ret_cond,     // RET Z
    [0xD0] = op_ret_cond,     // RET NC
    [0xD8] = op_ret_cond,     // RET C
    // Type: RET
    [0xC9] = op_ret,
    // Type: RETI
    [0xD9] = op_reti,
    // Type: JP cond, imm16
    [0xC2] = op_jp_cond_imm16, // JP NZ,imm16
    [0xCA] = op_jp_cond_imm16, // JP Z,imm16
    [0xD2] = op_jp_cond_imm16, // JP NC,imm16
    [0xDA] = op_jp_cond_imm16, // JP C,imm16
    // Type: JP imm16
    [0xC3] = op_jp_imm16,
    // Type: JP hl
    [0xE9] = op_jp_hl,
    // Type: CALL cond, imm16
    [0xC4] = op_call_cond_imm16, // CALL NZ,imm16
    [0xCC] = op_call_cond_imm16, // CALL Z,imm16
    [0xD4] = op_call_cond_imm16, // CALL NC,imm16
    [0xDC] = op_call_cond_imm16, // CALL C,imm16
    // Type: CALL imm16
    [0xCD] = op_call_imm16,
    // Type: RST tgt3
    [0xC7] = op_rst_tgt3,        // RST $00
    [0xCF] = op_rst_tgt3,        // RST $08
    [0xD7] = op_rst_tgt3,        // RST $10
    [0xDF] = op_rst_tgt3,        // RST $18
    [0xE7] = op_rst_tgt3,        // RST $20
    [0xEF] = op_rst_tgt3,        // RST $28
    [0xF7] = op_rst_tgt3,        // RST $30
    [0xFF] = op_rst_tgt3,        // RST $38
    // Type: POP r16stk
    [0xC1] = op_pop_r16stk,      // POP BC
    [0xD1] = op_pop_r16stk,      // POP DE
    [0xE1] = op_pop_r16stk,      // POP HL
    [0xF1] = op_pop_r16stk,      // POP AF
    // Type: PUSH r16stk
    [0xC5] = op_push_r16stk,     // PUSH BC
    [0xD5] = op_push_r16stk,     // PUSH DE
    [0xE5] = op_push_r16stk,     // PUSH HL
    [0xF5] = op_push_r16stk,     // PUSH AF
    // Type: LDH [imm8], a
    [0xE0] = op_ldh_imm8mem_a,
    // Type: LDH [c], a
    [0xE2] = op_ldh_c_mem_a,
    // Type: LD [imm16], a
    [0xEA] = op_ld_imm16mem_a,
    // Type: LDH a, [imm8]
    [0xF0] = op_ldh_a_imm8mem,
    // Type: LDH a, [c]
    [0xF2] = op_ldh_a_c_mem,
    // Type: LD a, [imm16]
    [0xFA] = op_ld_a_imm16mem,
    // Type: ADD sp, imm8
    [0xE8] = op_add_sp_imm8,
    // Type: LD hl, sp + imm8
    [0xF8] = op_ld_hl_sp_plus_imm8,
    // Type: LD sp, hl
    [0xF9] = op_ld_sp_hl,
    // ... (initialize other opcodes as needed)
};
