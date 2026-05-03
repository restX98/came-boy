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

    return UNITY_END();
}
