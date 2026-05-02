#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "opcodes.h"

static cpu_t mock_cpu = { 0 };
static bus_t mock_bus = { 0 };
static uint8_t mock_memory[0x4000] = { 0 }; // 16 KiB, covers all the addresses accessed by the opcodes being tested

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

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_op_nop);
    RUN_TEST(test_op_ld_bc_d16);
    RUN_TEST(test_op_ld_de_d16);
    RUN_TEST(test_op_ld_hl_d16);
    RUN_TEST(test_op_ld_sp_d16);

    return UNITY_END();
}
