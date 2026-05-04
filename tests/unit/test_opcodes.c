#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "opcodes.h"

static cpu_t mock_cpu = { 0 };
static bus_t mock_bus = { 0 };
static uint8_t mock_memory[0xFFFF] = { 0 };

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

// ---- op_rra ----
void test_op_rra_inserts_carry_and_updates_lsb_to_flag(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0b11001010;

    uint8_t opcode = 0x1F; // RRA

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL(0b11100101, mock_cpu.af.hi);
}

void test_op_rra_clears_z_n_h_flags(void) {
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.af.hi = 0b11001010;

    uint8_t opcode = 0x1F; // RRA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_rra_sets_carry_flag_when_lsb_is_1(void) {
    flag_clear(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0b01100101;

    uint8_t opcode = 0x1F; // RRA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b00110010, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rra_clears_carry_flag_when_lsb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0b11010010;

    uint8_t opcode = 0x1F; // RRA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(0b11101001, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_daa ----

// After addition
void test_op_daa_no_adjustment_needed(void) {
    // 0x05 + 0x03 = 0x08, valid BCD, no adjustment needed
    mock_cpu.af.hi = 0x08;

    uint8_t opcode = 0x27; // DAA

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x08, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_daa_adjusts_lower_nibble_after_addition(void) {
    // 0x05 + 0x07 = 0x0C, lower nibble > 9, adjust by +0x06
    mock_cpu.af.hi = 0x0C;

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x12, mock_cpu.af.hi); // 0x0C + 0x06 = 0x12
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_daa_adjusts_upper_nibble_after_addition(void) {
    // result > 0x99, needs upper nibble adjustment
    mock_cpu.af.hi = 0xA0;

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi); // 0xA0 + 0x60 = 0x100, wraps to 0x00
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_daa_adjusts_both_nibbles_after_addition(void) {
    // lower nibble > 9 and upper nibble > 9
    mock_cpu.af.hi = 0xAC;

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x12, mock_cpu.af.hi); // 0xAC + 0x66 = 0x112, wraps to 0x12
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_daa_uses_carry_flag_to_adjust_upper_nibble(void) {
    // carry was set by previous addition
    mock_cpu.af.hi = 0x05;
    flag_set(&mock_cpu, FLAG_C);

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x65, mock_cpu.af.hi); // 0x05 + 0x60 = 0x65
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_daa_uses_half_carry_flag_to_adjust_lower_nibble(void) {
    // half carry was set by previous addition
    mock_cpu.af.hi = 0x10;
    flag_set(&mock_cpu, FLAG_H);

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x16, mock_cpu.af.hi); // 0x10 + 0x06 = 0x16
}

// After subtraction
void test_op_daa_adjusts_lower_nibble_after_subtraction(void) {
    // half carry set after subtraction
    mock_cpu.af.hi = 0x19;
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x13, mock_cpu.af.hi); // 0x19 - 0x06 = 0x13
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_daa_adjusts_upper_nibble_after_subtraction(void) {
    // carry set after subtraction
    mock_cpu.af.hi = 0x73;
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_C);

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x13, mock_cpu.af.hi); // 0x73 - 0x60 = 0x13
}

// Z flag
void test_op_daa_sets_z_flag_when_result_is_zero(void) {
    mock_cpu.af.hi = 0xA0; // will become 0x00 after adjustment

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_daa_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z); // Z set beforehand
    mock_cpu.af.hi = 0x08;

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

// H flag
void test_op_daa_always_clears_h_flag(void) {
    flag_set(&mock_cpu, FLAG_H); // H set beforehand
    mock_cpu.af.hi = 0x08;

    uint8_t opcode = 0x27; // DAA

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_cpl ----
void test_op_cpl(void) {
    mock_cpu.af.hi = 0b11001010;

    uint8_t opcode = 0x2F; // CPL

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0b00110101, mock_cpu.af.hi);
}


void test_op_cpl_sets_n_h_flags(void) {
    mock_cpu.af.hi = 0x42;

    uint8_t opcode = 0x2F; // CPL

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_cpl_all_zeros_becomes_all_ones(void) {
    mock_cpu.af.hi = 0x00;

    uint8_t opcode = 0x2F; // CPL

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);
}

void test_op_cpl_all_ones_becomes_all_zeros(void) {
    mock_cpu.af.hi = 0xFF;

    uint8_t opcode = 0x2F; // CPL

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
}

// ---- op_scf ----
void test_op_scf(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);

    uint8_t opcode = 0x37; // SCF

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_ccf ----
void test_op_ccf_clears_c_flag_when_set(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    flag_set(&mock_cpu, FLAG_C);

    uint8_t opcode = 0x3F; // CCF

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_ccf_sets_c_flag_when_clear(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    flag_clear(&mock_cpu, FLAG_C);

    uint8_t opcode = 0x3F; // CCF

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_jr_imm8 ----
void test_op_jr_imm8(void) {
    mock_cpu.pc = 0x0;
    mock_cpu.bc.reg = 0x1234;
    mock_cpu.de.reg = 0x5678;
    mock_cpu.hl.reg = 0x9ABC;
    mock_cpu.sp = 0xFFFE;
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0] = 0x00; // offset 0

    uint8_t opcode = 0x18; // JR imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(FLAG_Z, mock_cpu.af.lo & FLAG_Z);
    TEST_ASSERT_EQUAL_UINT8(FLAG_N, mock_cpu.af.lo & FLAG_N);
    TEST_ASSERT_EQUAL_UINT8(FLAG_H, mock_cpu.af.lo & FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(FLAG_C, mock_cpu.af.lo & FLAG_C);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.bc.reg);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.de.reg);
    TEST_ASSERT_EQUAL_UINT16(0x9ABC, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

void test_op_jr_imm8_zero_offset(void) {
    mock_memory[0] = 0x00; // offset 0

    uint8_t opcode = 0x18; // JR imm8

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // PC = 1 (after reading offset byte) + 0 = 1
    TEST_ASSERT_EQUAL_UINT16(0x0001, mock_cpu.pc);
}

void test_op_jr_imm8_positive_offset_jumps_forward(void) {
    mock_memory[0] = 0x05; // offset +5

    uint8_t opcode = 0x18; // JR imm8

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // PC = 1 (after reading offset byte) + 5 = 6
    TEST_ASSERT_EQUAL_UINT16(0x0006, mock_cpu.pc);
}

void test_op_jr_imm8_negative_offset_jumps_backward(void) {
    mock_cpu.pc = 0x10;
    mock_memory[0x10] = 0xFE; // offset -2 as int8_t

    uint8_t opcode = 0x18; // JR imm8

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // PC = 0x11 (after reading offset byte) + (-2) = 0x0F
    TEST_ASSERT_EQUAL_UINT16(0x000F, mock_cpu.pc);
}

// ---- op_jr_imm8 ----
void test_op_jr_nz_imm8_condition_true(void) {
    mock_cpu.pc = 0x0000;
    flag_clear(&mock_cpu, FLAG_Z); // NZ = true

    mock_memory[0] = 0x00; // offset 0

    uint8_t opcode = 0x20; // JR NZ, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc); // 1 + 0
}

void test_op_jr_nz_imm8_condition_false(void) {
    mock_cpu.pc = 0x0000;
    flag_set(&mock_cpu, FLAG_Z); // NZ = false

    mock_memory[0] = 0x05; // offset (ignored)

    uint8_t opcode = 0x20; // JR NZ, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc); // only consumed imm8
}

void test_op_jr_z_imm8_positive_offset(void) {
    mock_cpu.pc = 0x0000;
    flag_set(&mock_cpu, FLAG_Z); // Z = true

    mock_memory[0] = 0x05; // +5

    uint8_t opcode = 0x28; // JR Z, imm8

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // PC = 1 + 5 = 6
    TEST_ASSERT_EQUAL_UINT16(0x0006, mock_cpu.pc);
}

void test_op_jr_c_imm8_negative_offset(void) {
    mock_cpu.pc = 0x0010;
    flag_set(&mock_cpu, FLAG_C); // C = true

    mock_memory[0x10] = 0xFE; // -2

    uint8_t opcode = 0x38; // JR C, imm8

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // PC = 0x11 + (-2) = 0x0F
    TEST_ASSERT_EQUAL_UINT16(0x000F, mock_cpu.pc);
}

void test_op_jr_nc_imm8_negative_offset_condition_false(void) {
    mock_cpu.pc = 0x0010;
    flag_set(&mock_cpu, FLAG_C); // NC = false

    mock_memory[0x10] = 0xFE; // -2 (ignored)

    uint8_t opcode = 0x30; // JR NC, imm8

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // PC = 0x11 (no jump)
    TEST_ASSERT_EQUAL_UINT16(0x0011, mock_cpu.pc);
}

// ---- op_ld_r8_r8 ----
struct reg_entry_t {
    r8_operand_t code;
    uint8_t *reg;
} regs[] = {
    {OP_REG_B, &mock_cpu.bc.hi},
    {OP_REG_C, &mock_cpu.bc.lo},
    {OP_REG_D, &mock_cpu.de.hi},
    {OP_REG_E, &mock_cpu.de.lo},
    {OP_REG_H, &mock_cpu.hl.hi},
    {OP_REG_L, &mock_cpu.hl.lo},
    {OP_REG_A + 1, &mock_cpu.af.hi},
};

void test_op_ld_hl_mem_r8(void) {
    mock_memory[0x10] = 0xAA;

    uint8_t opcodes[7] = { 0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E, 0x7E };
    for (int i = 0; i < 7; i++) {
        mock_cpu.hl.reg = 0x10;

        uint8_t opcode = opcodes[i]; // LD r8,[HL]

        int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

        TEST_ASSERT_EQUAL(8, cycles);
        TEST_ASSERT_EQUAL_UINT8(0xAA, *regs[i].reg);
    }
}

void test_op_ld_r8_hl_mem(void) {
    uint8_t opcodes[7] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77 };

    for (int i = 0; i < 7; i++) {
        memset(mock_memory, 0, sizeof(mock_memory));
        mock_cpu.hl.reg = 0x20;
        *regs[i].reg = 0x55;

        uint8_t opcode = opcodes[i]; // LD [HL],r8

        int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

        // HL may have changed if r = H or L
        uint16_t expected_addr = mock_cpu.hl.reg;

        TEST_ASSERT_EQUAL(8, cycles);
        TEST_ASSERT_EQUAL_UINT8(0x55, mock_memory[expected_addr]);
    }
}

void test_op_ld_r8_r8_matrix(void) {
    uint8_t opcodes[7][7] = {
        {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x47},
        {0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4F},
        {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x57},
        {0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5F},
        {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x67},
        {0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6F},
        {0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7F},
    };

    for (int dst = 0; dst < 7; dst++) {
        for (int src = 0; src < 6; src++) {
            if (dst == src) continue;

            *regs[src].reg = 0x99;
            *regs[dst].reg = 0x00;

            uint8_t opcode = opcodes[dst][src];

            int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

            TEST_ASSERT_EQUAL(4, cycles);
            TEST_ASSERT_EQUAL_UINT8(0x99, *regs[dst].reg);
        }
    }
}

// ---- op_add_a_r8 ----
void test_op_add_a_r8(void) {
    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x22; // B

    uint8_t opcode = 0x80; // ADD A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x32, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_add_a_r8_sets_zero_flag(void) {
    mock_cpu.af.hi = 0x80;
    mock_cpu.bc.hi = 0x80; // B

    uint8_t opcode = 0x80; // ADD A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_add_a_r8_sets_half_carry_and_carry(void) {
    mock_cpu.af.hi = 0xFF;
    mock_cpu.bc.hi = 0x01; // B

    uint8_t opcode = 0x80; // ADD A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_add_a_hl_mem(void) {
    mock_cpu.af.hi = 0x10;
    mock_cpu.hl.reg = 0x20;

    mock_memory[0x20] = 0x22;

    uint8_t opcode = 0x86; // ADD A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x32, mock_cpu.af.hi);
}

void test_op_add_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0x80, &mock_cpu.bc.hi}, // B
        {0x81, &mock_cpu.bc.lo}, // C
        {0x82, &mock_cpu.de.hi}, // D
        {0x83, &mock_cpu.de.lo}, // E
        {0x84, &mock_cpu.hl.hi}, // H
        {0x85, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        mock_cpu.af.hi = 0x10;
        *cases[i].reg = 0x05;

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x15, mock_cpu.af.hi);
    }
}

void test_op_add_a_a(void) {
    mock_cpu.af.hi = 0x10;

    uint8_t opcode = 0x87; // ADD A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_adc_a_r8 ----
void test_op_adc_a_r8_no_carry_in(void) {
    flag_clear(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x22; // B

    uint8_t opcode = 0x88; // ADC A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x32, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_adc_a_r8_with_carry_in(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x22; // B

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x33, mock_cpu.af.hi);
}

void test_op_adc_a_r8_sets_zero_flag(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0xFF;
    mock_cpu.bc.hi = 0x00; // B

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_adc_a_r8_sets_half_carry_with_carry_in(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x0F;
    mock_cpu.bc.hi = 0x00; // B

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x10, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_adc_a_r8_sets_carry(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0xF0;
    mock_cpu.bc.hi = 0x0F; // B

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_adc_a_r8_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N);

    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x01;

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_adc_a_hl_mem(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0x22;

    uint8_t opcode = 0x8E; // ADC A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x33, mock_cpu.af.hi);
}

void test_op_adc_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0x88, &mock_cpu.bc.hi}, // B
        {0x89, &mock_cpu.bc.lo}, // C
        {0x8A, &mock_cpu.de.hi}, // D
        {0x8B, &mock_cpu.de.lo}, // E
        {0x8C, &mock_cpu.hl.hi}, // H
        {0x8D, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        flag_set(&mock_cpu, FLAG_C);

        mock_cpu.af.hi = 0x10;
        *cases[i].reg = 0x05;

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x16, mock_cpu.af.hi);
    }
}

void test_op_adc_a_a(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;

    uint8_t opcode = 0x8F; // ADC A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x21, mock_cpu.af.hi);
}

// ---- op_sub_a_r8 ----
void test_op_sub_a_r8(void) {
    mock_cpu.af.hi = 0x30;
    mock_cpu.bc.hi = 0x10; // B

    uint8_t opcode = 0x90; // SUB A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sub_a_r8_sets_zero_flag(void) {
    mock_cpu.af.hi = 0x42;
    mock_cpu.bc.hi = 0x42; // B

    uint8_t opcode = 0x90; // SUB A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

void test_op_sub_a_r8_sets_half_borrow_and_carry(void) {
    mock_cpu.af.hi = 0x00;
    mock_cpu.bc.hi = 0x01; // B

    uint8_t opcode = 0x90; // SUB A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sub_a_hl_mem(void) {
    mock_cpu.af.hi = 0x30;
    mock_cpu.hl.reg = 0x20;

    mock_memory[0x20] = 0x10;

    uint8_t opcode = 0x96; // SUB A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);
}

void test_op_sub_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0x90, &mock_cpu.bc.hi}, // B
        {0x91, &mock_cpu.bc.lo}, // C
        {0x92, &mock_cpu.de.hi}, // D
        {0x93, &mock_cpu.de.lo}, // E
        {0x94, &mock_cpu.hl.hi}, // H
        {0x95, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        mock_cpu.af.hi = 0x20;
        *cases[i].reg = 0x05;

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x1B, mock_cpu.af.hi);
    }
}

void test_op_sub_a_a(void) {
    mock_cpu.af.hi = 0x55;

    uint8_t opcode = 0x97; // SUB A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
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
    RUN_TEST(test_op_rra_inserts_carry_and_updates_lsb_to_flag);
    RUN_TEST(test_op_rra_clears_z_n_h_flags);
    RUN_TEST(test_op_rra_sets_carry_flag_when_lsb_is_1);
    RUN_TEST(test_op_rra_clears_carry_flag_when_lsb_is_0);
    RUN_TEST(test_op_daa_no_adjustment_needed);
    RUN_TEST(test_op_daa_adjusts_lower_nibble_after_addition);
    RUN_TEST(test_op_daa_adjusts_upper_nibble_after_addition);
    RUN_TEST(test_op_daa_adjusts_both_nibbles_after_addition);
    RUN_TEST(test_op_daa_uses_carry_flag_to_adjust_upper_nibble);
    RUN_TEST(test_op_daa_uses_half_carry_flag_to_adjust_lower_nibble);
    RUN_TEST(test_op_daa_adjusts_lower_nibble_after_subtraction);
    RUN_TEST(test_op_daa_adjusts_upper_nibble_after_subtraction);
    RUN_TEST(test_op_daa_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_daa_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_daa_always_clears_h_flag);
    RUN_TEST(test_op_cpl);
    RUN_TEST(test_op_cpl_sets_n_h_flags);
    RUN_TEST(test_op_cpl_all_zeros_becomes_all_ones);
    RUN_TEST(test_op_cpl_all_ones_becomes_all_zeros);
    RUN_TEST(test_op_scf);
    RUN_TEST(test_op_ccf_clears_c_flag_when_set);
    RUN_TEST(test_op_ccf_sets_c_flag_when_clear);
    RUN_TEST(test_op_jr_imm8);
    RUN_TEST(test_op_jr_imm8_zero_offset);
    RUN_TEST(test_op_jr_imm8_positive_offset_jumps_forward);
    RUN_TEST(test_op_jr_imm8_negative_offset_jumps_backward);
    RUN_TEST(test_op_jr_nz_imm8_condition_true);
    RUN_TEST(test_op_jr_nz_imm8_condition_false);
    RUN_TEST(test_op_jr_z_imm8_positive_offset);
    RUN_TEST(test_op_jr_c_imm8_negative_offset);
    RUN_TEST(test_op_jr_nc_imm8_negative_offset_condition_false);
    RUN_TEST(test_op_ld_hl_mem_r8);
    RUN_TEST(test_op_ld_r8_hl_mem);
    RUN_TEST(test_op_ld_r8_r8_matrix);
    RUN_TEST(test_op_add_a_r8);
    RUN_TEST(test_op_add_a_r8_sets_zero_flag);
    RUN_TEST(test_op_add_a_r8_sets_half_carry_and_carry);
    RUN_TEST(test_op_add_a_hl_mem);
    RUN_TEST(test_op_add_a_r8_all_registers);
    RUN_TEST(test_op_add_a_a);
    RUN_TEST(test_op_adc_a_r8_no_carry_in);
    RUN_TEST(test_op_adc_a_r8_with_carry_in);
    RUN_TEST(test_op_adc_a_r8_sets_zero_flag);
    RUN_TEST(test_op_adc_a_r8_sets_half_carry_with_carry_in);
    RUN_TEST(test_op_adc_a_r8_sets_carry);
    RUN_TEST(test_op_adc_a_r8_clears_n_flag);
    RUN_TEST(test_op_adc_a_hl_mem);
    RUN_TEST(test_op_adc_a_r8_all_registers);
    RUN_TEST(test_op_adc_a_a);
    RUN_TEST(test_op_sub_a_r8);
    RUN_TEST(test_op_sub_a_r8_sets_zero_flag);
    RUN_TEST(test_op_sub_a_r8_sets_half_borrow_and_carry);
    RUN_TEST(test_op_sub_a_hl_mem);
    RUN_TEST(test_op_sub_a_r8_all_registers);
    RUN_TEST(test_op_sub_a_a);

    return UNITY_END();
}
