#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "opcodes.h"

static cpu_t mock_cpu = { 0 };
static bus_t mock_bus = { 0 };
static uint8_t mock_memory[0x10] = { 0 };

void setUp(void) {
    suppress_logs();
}

void tearDown(void) {
    mock_cpu = (cpu_t){ 0 }; // Reset CPU state
    mock_bus = (bus_t){ 0 }; // Reset bus state
    memset(mock_memory, 0, sizeof(mock_memory)); // Clear mock memory after each test
    restore_logs();
}

// ---- Mock functions ----

uint8_t bus_read(bus_t *bus, uint16_t addr) {
    (void)bus; // Unused parameter
    return mock_memory[addr];
}

void bus_write(bus_t *bus, uint16_t addr, uint8_t value) {
    (void)bus; (void)addr; (void)value;
    mock_memory[addr] = value;
}

// ---- op_nop ----
void test_op_nop(void) {
    uint8_t opcode = 0x00; // NOP

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc); // PC should not change
}

// ---- op_ld_r16_imm16 ----
void test_op_ld_bc_imm16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x01; // LD BC, imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(2, mock_cpu.pc); // PC should advance by 2 for the immediate value
    TEST_ASSERT_EQUAL(0x1234, mock_cpu.bc.reg); // BC should be loaded with 0x1234
}

void test_op_ld_de_imm16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x11; // LD DE, imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(2, mock_cpu.pc); // PC should advance by 2 for the immediate value
    TEST_ASSERT_EQUAL(0x1234, mock_cpu.de.reg); // DE should be loaded with 0x1234
}

void test_op_ld_hl_imm16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x21; // LD HL, imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(2, mock_cpu.pc); // PC should advance by 2 for the immediate value
    TEST_ASSERT_EQUAL(0x1234, mock_cpu.hl.reg); // HL should be loaded with 0x1234
}

void test_op_ld_sp_imm16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x31; // LD SP, imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(2, mock_cpu.pc); // PC should advance by 2 for the immediate value
    TEST_ASSERT_EQUAL(0x1234, mock_cpu.sp); // SP should be loaded with 0x1234
}

// ---- op_ld_r16mem_a ----
void test_op_ld_bc_mem_a(void) {
    uint8_t value = 0x42;

    mock_cpu.bc.reg = 0x03;
    mock_cpu.af.hi = value;

    uint8_t opcode = 0x02; // LD [BC], A

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x03, mock_cpu.bc.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(mock_cpu.af.hi, value);
}

void test_op_ld_de_mem_a(void) {
    uint8_t value = 0x42;

    mock_cpu.de.reg = 0x03;
    mock_cpu.af.hi = value;

    uint8_t opcode = 0x12; // LD [DE], A

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x03, mock_cpu.de.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(mock_cpu.af.hi, value);
}

void test_op_ld_hli_mem_a(void) {
    uint8_t value = 0x42;

    mock_cpu.hl.reg = 0x03;
    mock_cpu.af.hi = value;

    uint8_t opcode = 0x22; // LD [HL+], A

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x04, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(mock_cpu.af.hi, value);
}

void test_op_ld_hld_mem_a(void) {
    uint8_t value = 0x42;

    mock_cpu.hl.reg = 0x03;
    mock_cpu.af.hi = value;

    uint8_t opcode = 0x32; // LD [HL-], A

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x02, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(mock_cpu.af.hi, value);
}

// ---- op_ld_a_r16mem ----
void test_op_ld_a_bc_mem(void) {
    uint8_t value = 0x42;

    mock_cpu.bc.reg = 0x03;
    mock_memory[mock_cpu.bc.reg] = value;

    uint8_t opcode = 0x0A; // LD A, [BC]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x03, mock_cpu.bc.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(value, mock_cpu.af.hi);
}

void test_op_ld_a_de_mem(void) {
    uint8_t value = 0x42;

    mock_cpu.de.reg = 0x03;
    mock_memory[mock_cpu.de.reg] = value;

    uint8_t opcode = 0x1A; // LD A, [DE]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x03, mock_cpu.de.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(value, mock_cpu.af.hi);
}

void test_op_ld_a_hli_mem(void) {
    uint8_t value = 0x42;

    mock_cpu.hl.reg = 0x03;
    mock_memory[mock_cpu.hl.reg] = value;

    uint8_t opcode = 0x2A; // LD A, [HL+]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x04, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(value, mock_cpu.af.hi);
}

void test_op_ld_a_hld_mem(void) {
    uint8_t value = 0x42;

    mock_cpu.hl.reg = 0x03;
    mock_memory[mock_cpu.hl.reg] = value;

    uint8_t opcode = 0x3A; // LD A, [HL-]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x02, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(value, mock_cpu.af.hi);
}

// ---- op_ld_imm16mem_sp ----
void test_op_ld_imm16mem_sp(void) {
    mock_cpu.sp = 0x1234;
    mock_memory[0] = 0x05; // Low byte of target address
    mock_memory[1] = 0x00; // High byte of target address

    uint8_t opcode = 0x08; // LD [n16], SP

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(20, cycles);
    TEST_ASSERT_EQUAL_UINT16(2, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x34, mock_memory[0x0005]); // Save cpu->sp >> 8 at address
    TEST_ASSERT_EQUAL_UINT16(0x12, mock_memory[0x0006]); // Save cpu->sp & 0xFF at adress+1
}

// ---- op_inc_r16 ----
void test_op_inc_bc(void) {
    mock_cpu.bc.reg = 0x10;

    uint8_t opcode = 0x03; // INC BC

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x11, mock_cpu.bc.reg);
}

void test_op_inc_de(void) {
    mock_cpu.de.reg = 0x10;

    uint8_t opcode = 0x13; // INC DE

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x11, mock_cpu.de.reg);
}

void test_op_inc_hl(void) {
    mock_cpu.hl.reg = 0x10;

    uint8_t opcode = 0x23; // INC HL

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x11, mock_cpu.hl.reg);
}

void test_op_inc_sp(void) {
    mock_cpu.sp = 0x10;

    uint8_t opcode = 0x33; // INC SP

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x11, mock_cpu.sp);
}

// ---- op_dec_r16 ----
void test_op_dec_bc(void) {
    mock_cpu.bc.reg = 0x10;

    uint8_t opcode = 0x0B; // INC BC

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0F, mock_cpu.bc.reg);
}

void test_op_dec_de(void) {
    mock_cpu.de.reg = 0x10;

    uint8_t opcode = 0x1B; // INC DE

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0F, mock_cpu.de.reg);
}

void test_op_dec_hl(void) {
    mock_cpu.hl.reg = 0x10;

    uint8_t opcode = 0x2B; // INC HL

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0F, mock_cpu.hl.reg);
}

void test_op_dec_sp(void) {
    mock_cpu.sp = 0x10;

    uint8_t opcode = 0x3B; // INC SP

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0F, mock_cpu.sp);
}

// ---- op_add_hl_r16 ----
void test_op_add_hl_bc(void) {
    mock_cpu.bc.reg = 0x06;
    mock_cpu.hl.reg = 0x03;

    uint8_t opcode = 0x09; // ADD HL, BC

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x09, mock_cpu.hl.reg);
}

void test_op_add_hl_de(void) {
    mock_cpu.de.reg = 0x06;
    mock_cpu.hl.reg = 0x03;

    uint8_t opcode = 0x19; // ADD HL, DE

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x09, mock_cpu.hl.reg);
}

void test_op_add_hl_hl(void) {
    mock_cpu.hl.reg = 0x03;

    uint8_t opcode = 0x29; // ADD HL, HL

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x06, mock_cpu.hl.reg);
}

void test_op_add_hl_sp(void) {
    mock_cpu.sp = 0x06;
    mock_cpu.hl.reg = 0x03;

    uint8_t opcode = 0x39; // ADD HL, SP

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x09, mock_cpu.hl.reg);
}

void test_op_add_hl_r16_reset_nhc_flags_if_no_overflow(void) {
    // 0000000000000101+
    // 0000000000000011=
    // 0001000000001000
    mock_cpu.bc.reg = 0x0005;
    mock_cpu.hl.reg = 0x0003;
    flag_set(&mock_cpu, FLAG_N); // N set beforehand

    uint8_t opcode = 0x09; // ADD HL, BC

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x0008, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_add_hl_r16_set_half_carry_if_overflow_from_bit_11(void) {
    // 0000111111111111+
    // 0000000000000001=
    // 0001000000000000
    mock_cpu.bc.reg = 0x0FFF;
    mock_cpu.hl.reg = 0x0001;

    uint8_t opcode = 0x09; // ADD HL, BC

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x1000, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_add_hl_r16_set_carry_if_overflow_from_bit_15(void) {
    // 1111111111111111+
    // 0000000000000001=
    // 0000000000000000
    mock_cpu.bc.reg = 0xFFFF;
    mock_cpu.hl.reg = 0x0001;

    uint8_t opcode = 0x09; // ADD HL, BC

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x0000, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_inc_r8 ----
void test_op_inc_b(void) {
    mock_cpu.bc.hi = 0x2;

    uint8_t opcode = 0x04; // INC B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_cpu.bc.hi);
}

void test_op_inc_c(void) {
    mock_cpu.bc.lo = 0x2;

    uint8_t opcode = 0x0C; // INC C

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_cpu.bc.lo);
}

void test_op_inc_d(void) {
    mock_cpu.de.hi = 0x2;

    uint8_t opcode = 0x14; // INC D

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_cpu.de.hi);
}

void test_op_inc_e(void) {
    mock_cpu.de.lo = 0x2;

    uint8_t opcode = 0x1C; // INC E

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_cpu.de.lo);
}


void test_op_inc_h(void) {
    mock_cpu.hl.hi = 0x2;

    uint8_t opcode = 0x24; // INC H

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_cpu.hl.hi);
}

void test_op_inc_l(void) {
    mock_cpu.hl.lo = 0x2;

    uint8_t opcode = 0x2C; // INC L

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_cpu.hl.lo);
}

void test_op_inc_hl_mem(void) {
    mock_cpu.hl.reg = 0x0;
    mock_memory[mock_cpu.hl.reg] = 0x2;

    uint8_t opcode = 0x34; // INC [HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_memory[mock_cpu.hl.reg]);
}

void test_op_inc_a(void) {
    mock_cpu.af.hi = 0x2;

    uint8_t opcode = 0x3C; // INC A

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_cpu.af.hi);
}

void test_op_inc_r8_sets_z_flag_on_overflow(void) {
    mock_cpu.bc.hi = 0xFF;

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_inc_r8_sets_h_flag_on_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x0F; // 00001111 -> 00010000

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x10, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_inc_r8_does_not_set_h_flag_when_no_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x10; // 00010000 -> 00010001

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x11, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(false, flag_get(&mock_cpu, FLAG_H));
}

void test_op_inc_r8_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N); // N set beforehand
    mock_cpu.bc.hi = 0x01;

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

// ---- op_dec_r8 ----
void test_op_dec_b(void) {
    mock_cpu.bc.hi = 0x2;

    uint8_t opcode = 0x05; // DEC B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_cpu.bc.hi);
}

void test_op_dec_c(void) {
    mock_cpu.bc.lo = 0x2;

    uint8_t opcode = 0x0D; // DEC C

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_cpu.bc.lo);
}

void test_op_dec_d(void) {
    mock_cpu.de.hi = 0x2;

    uint8_t opcode = 0x15; // DEC D

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_cpu.de.hi);
}

void test_op_dec_e(void) {
    mock_cpu.de.lo = 0x2;

    uint8_t opcode = 0x1D; // DEC E

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_cpu.de.lo);
}


void test_op_dec_h(void) {
    mock_cpu.hl.hi = 0x2;

    uint8_t opcode = 0x25; // DEC H

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_cpu.hl.hi);
}

void test_op_dec_l(void) {
    mock_cpu.hl.lo = 0x2;

    uint8_t opcode = 0x2D; // DEC L

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_cpu.hl.lo);
}

void test_op_dec_hl_mem(void) {
    mock_cpu.hl.reg = 0x0;
    mock_memory[mock_cpu.hl.reg] = 0x2;

    uint8_t opcode = 0x35; // DEC [HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_memory[mock_cpu.hl.reg]);
}

void test_op_dec_a(void) {
    mock_cpu.af.hi = 0x2;

    uint8_t opcode = 0x3D; // DEC A

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_cpu.af.hi);
}

void test_op_dec_r8_sets_z_flag_on_overflow(void) {
    mock_cpu.bc.hi = 0x01;

    uint8_t opcode = 0x05; // DEC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_dec_r8_sets_h_flag_on_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x10; // 00010000 -> 00001111

    uint8_t opcode = 0x05; // DEC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x0F, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_dec_r8_does_not_set_h_flag_when_no_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x08; // 00001000 -> 00000111

    uint8_t opcode = 0x05; // DEC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x07, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(false, flag_get(&mock_cpu, FLAG_H));
}

void test_op_dec_r8_wraps_and_sets_h(void) {
    mock_cpu.bc.hi = 0x00;

    uint8_t opcode = 0x05;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_dec_r8_sets_n_flag(void) {
    flag_clear(&mock_cpu, FLAG_N); // N clear beforehand
    mock_cpu.bc.hi = 0x01;

    uint8_t opcode = 0x05; // DEC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

// ---- op_ld_r8_imm8 ----
void test_op_ld_b_imm8(void) {
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x06; // LD B, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_cpu.bc.hi);
}

void test_op_ld_c_imm8(void) {
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x0E; // LD C, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_cpu.bc.lo);
}

void test_op_ld_d_imm8(void) {
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x16; // LD D, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_cpu.de.hi);
}

void test_op_ld_e_imm8(void) {
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x1E; // LD E, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_cpu.de.lo);
}

void test_op_ld_h_imm8(void) {
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x26; // LD H, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_cpu.hl.hi);
}

void test_op_ld_l_imm8(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x2E; // LD L, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_cpu.hl.lo);
}

void test_op_ld_hl_mem_imm8(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_cpu.hl.reg = 0x05;
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x36; // LD [HL], imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_memory[mock_cpu.hl.reg]);
}

void test_op_ld_a_imm8(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x12; // Immediate value

    uint8_t opcode = 0x3E; // LD A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0x12, mock_cpu.af.hi);
}

// ---- op_rlca ----
void test_op_rlca(void) {
    mock_cpu.af.hi = 0b01100101;

    uint8_t opcode = 0x07; // RLCA

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0b11001010, mock_cpu.af.hi);
}

void test_op_rlca_clears_z_n_h_flags(void) {
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.af.hi = 0b01100101;

    uint8_t opcode = 0x07; // RLCA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_rlca_sets_carry_flag_when_msb_is_1(void) {
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.af.hi = 0b10100101;

    uint8_t opcode = 0x07; // RLCA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b01001011, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rlca_clears_carry_flag_when_msb_is_0(void) {
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.af.hi = 0b01100101;

    uint8_t opcode = 0x07; // RLCA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b11001010, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_rrca ----
void test_op_rrca(void) {
    mock_cpu.af.hi = 0b01100101;

    uint8_t opcode = 0x0F; // RRCA

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0b10110010, mock_cpu.af.hi);
}

void test_op_rrca_clears_z_n_h_flags(void) {
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.af.hi = 0b01100101;

    uint8_t opcode = 0x0F; // RRCA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_rrca_sets_carry_flag_when_lsb_is_1(void) {
    mock_cpu.af.hi = 0b10100101;

    uint8_t opcode = 0x0F; // RRCA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b11010010, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rrca_clears_carry_flag_when_lsb_is_0(void) {
    mock_cpu.af.hi = 0b11001010;

    uint8_t opcode = 0x0F; // RRCA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b01100101, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_rla ----
void test_op_rla_inserts_carry_and_updates_msb_to_flag(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0b11001010;

    uint8_t opcode = 0x17; // RLA

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0b10010101, mock_cpu.af.hi);
}

void test_op_rla_clears_z_n_h_flags(void) {
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.af.hi = 0b11001010;

    uint8_t opcode = 0x17; // RLA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_rla_sets_carry_flag_when_msb_is_1(void) {
    flag_clear(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0b10100100;

    uint8_t opcode = 0x17; // RLA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b01001000, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rla_clears_carry_flag_when_msb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0b01010010;

    uint8_t opcode = 0x17; // RLA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b10100101, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_op_nop);
    RUN_TEST(test_op_ld_bc_imm16);
    RUN_TEST(test_op_ld_de_imm16);
    RUN_TEST(test_op_ld_hl_imm16);
    RUN_TEST(test_op_ld_sp_imm16);
    RUN_TEST(test_op_ld_bc_mem_a);
    RUN_TEST(test_op_ld_de_mem_a);
    RUN_TEST(test_op_ld_hli_mem_a);
    RUN_TEST(test_op_ld_hld_mem_a);
    RUN_TEST(test_op_ld_a_bc_mem);
    RUN_TEST(test_op_ld_a_de_mem);
    RUN_TEST(test_op_ld_a_hli_mem);
    RUN_TEST(test_op_ld_a_hld_mem);
    RUN_TEST(test_op_ld_imm16mem_sp);
    RUN_TEST(test_op_inc_bc);
    RUN_TEST(test_op_inc_de);
    RUN_TEST(test_op_inc_hl);
    RUN_TEST(test_op_inc_sp);
    RUN_TEST(test_op_dec_bc);
    RUN_TEST(test_op_dec_de);
    RUN_TEST(test_op_dec_hl);
    RUN_TEST(test_op_dec_sp);
    RUN_TEST(test_op_add_hl_bc);
    RUN_TEST(test_op_add_hl_de);
    RUN_TEST(test_op_add_hl_hl);
    RUN_TEST(test_op_add_hl_sp);
    RUN_TEST(test_op_add_hl_r16_reset_nhc_flags_if_no_overflow);
    RUN_TEST(test_op_add_hl_r16_set_half_carry_if_overflow_from_bit_11);
    RUN_TEST(test_op_add_hl_r16_set_carry_if_overflow_from_bit_15);
    RUN_TEST(test_op_inc_b);
    RUN_TEST(test_op_inc_c);
    RUN_TEST(test_op_inc_d);
    RUN_TEST(test_op_inc_e);
    RUN_TEST(test_op_inc_h);
    RUN_TEST(test_op_inc_l);
    RUN_TEST(test_op_inc_hl_mem);
    RUN_TEST(test_op_inc_a);
    RUN_TEST(test_op_inc_r8_sets_z_flag_on_overflow);
    RUN_TEST(test_op_inc_r8_sets_h_flag_on_nibble_overflow);
    RUN_TEST(test_op_inc_r8_does_not_set_h_flag_when_no_nibble_overflow);
    RUN_TEST(test_op_inc_r8_clears_n_flag);
    RUN_TEST(test_op_dec_b);
    RUN_TEST(test_op_dec_c);
    RUN_TEST(test_op_dec_d);
    RUN_TEST(test_op_dec_e);
    RUN_TEST(test_op_dec_h);
    RUN_TEST(test_op_dec_l);
    RUN_TEST(test_op_dec_hl_mem);
    RUN_TEST(test_op_dec_a);
    RUN_TEST(test_op_dec_r8_sets_z_flag_on_overflow);
    RUN_TEST(test_op_dec_r8_sets_h_flag_on_nibble_overflow);
    RUN_TEST(test_op_dec_r8_does_not_set_h_flag_when_no_nibble_overflow);
    RUN_TEST(test_op_dec_r8_wraps_and_sets_h);
    RUN_TEST(test_op_dec_r8_sets_n_flag);
    RUN_TEST(test_op_ld_b_imm8);
    RUN_TEST(test_op_ld_c_imm8);
    RUN_TEST(test_op_ld_d_imm8);
    RUN_TEST(test_op_ld_e_imm8);
    RUN_TEST(test_op_ld_h_imm8);
    RUN_TEST(test_op_ld_l_imm8);
    RUN_TEST(test_op_ld_hl_mem_imm8);
    RUN_TEST(test_op_ld_a_imm8);
    RUN_TEST(test_op_rlca);
    RUN_TEST(test_op_rlca_clears_z_n_h_flags);
    RUN_TEST(test_op_rlca_sets_carry_flag_when_msb_is_1);
    RUN_TEST(test_op_rlca_clears_carry_flag_when_msb_is_0);
    RUN_TEST(test_op_rrca);
    RUN_TEST(test_op_rrca_clears_z_n_h_flags);
    RUN_TEST(test_op_rrca_sets_carry_flag_when_lsb_is_1);
    RUN_TEST(test_op_rrca_clears_carry_flag_when_lsb_is_0);
    RUN_TEST(test_op_rla_inserts_carry_and_updates_msb_to_flag);
    RUN_TEST(test_op_rla_clears_z_n_h_flags);
    RUN_TEST(test_op_rla_sets_carry_flag_when_msb_is_1);
    RUN_TEST(test_op_rla_clears_carry_flag_when_msb_is_0);

    return UNITY_END();
}
