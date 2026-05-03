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

// ---- op_ld_r16_d16 ----
void test_op_ld_bc_d16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x01; // LD BC, d16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(2, mock_cpu.pc); // PC should advance by 2 for the immediate value
    TEST_ASSERT_EQUAL(0x1234, mock_cpu.bc.reg); // BC should be loaded with 0x1234
}

void test_op_ld_de_d16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x11; // LD DE, d16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(2, mock_cpu.pc); // PC should advance by 2 for the immediate value
    TEST_ASSERT_EQUAL(0x1234, mock_cpu.de.reg); // DE should be loaded with 0x1234
}

void test_op_ld_hl_d16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x21; // LD HL, d16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL(2, mock_cpu.pc); // PC should advance by 2 for the immediate value
    TEST_ASSERT_EQUAL(0x1234, mock_cpu.hl.reg); // HL should be loaded with 0x1234
}

void test_op_ld_sp_d16(void) {
    // Use mock memory to simulate the ROM contents for the opcode and its immediate value
    mock_memory[0] = 0x34; // Low byte of immediate value
    mock_memory[1] = 0x12; // High byte of immediate value

    uint8_t opcode = 0x31; // LD SP, d16

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

    uint8_t opcode = 0x09; // ADD HL, BC

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x0008, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT8(0, mock_cpu.af.lo & FLAG_N);
    TEST_ASSERT_EQUAL_UINT8(0, mock_cpu.af.lo & FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(0, mock_cpu.af.lo & FLAG_C);
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
    TEST_ASSERT_EQUAL_UINT8(FLAG_H, mock_cpu.af.lo & FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(0, mock_cpu.af.lo & FLAG_C);
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
    TEST_ASSERT_EQUAL_UINT8(FLAG_H, mock_cpu.af.lo & FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(FLAG_C, mock_cpu.af.lo & FLAG_C);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_op_nop);
    RUN_TEST(test_op_ld_bc_d16);
    RUN_TEST(test_op_ld_de_d16);
    RUN_TEST(test_op_ld_hl_d16);
    RUN_TEST(test_op_ld_sp_d16);
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

    return UNITY_END();
}
