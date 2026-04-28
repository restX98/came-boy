#include "unity.h"
#include "test_helpers.h"
#include "cpu.h"


void setUp(void) {
    suppress_logs();
}

void tearDown(void) {
    restore_logs();
}

// ---- Helper functions ----
static void setup_cpu_with_opcode(cpu_t *cpu, bus_t *bus, cartridge_t *cartridge, uint8_t opcode) {
    static uint8_t fake_rom[1];
    fake_rom[0] = opcode;
    *cartridge = (cartridge_t){ .rom = fake_rom, .size = 1, .bank = 1 };
    *bus = (bus_t){ .cartridge = cartridge };
    *cpu = (cpu_t){ 0 };
    cpu->pc = 0x0000;
}

// ---- cpu_init ----

void test_cpu_init_sets_registers_to_initial_values(void) {
    cpu_t cpu;
    cpu_init(&cpu);

    TEST_ASSERT_EQUAL_UINT16(0x01B0, cpu.af.reg);
    TEST_ASSERT_EQUAL_UINT16(0x0013, cpu.bc.reg);
    TEST_ASSERT_EQUAL_UINT16(0x00D8, cpu.de.reg);
    TEST_ASSERT_EQUAL_UINT16(0x014D, cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(0x0100, cpu.pc);
}

// ---- cpu_step ----

void test_cpu_step_returns_minus1_on_unknown_opcode(void) {
    cpu_t cpu; bus_t bus; cartridge_t cartridge;
    setup_cpu_with_opcode(&cpu, &bus, &cartridge, 0xD3);
    TEST_ASSERT_EQUAL_INT(-1, cpu_step(&cpu, &bus));
}

void test_cpu_step_executes_nop_correctly(void) {
    cpu_t cpu; bus_t bus; cartridge_t cartridge;
    setup_cpu_with_opcode(&cpu, &bus, &cartridge, 0x00);
    TEST_ASSERT_EQUAL_INT(4, cpu_step(&cpu, &bus));
}

void test_cpu_step_nop_increments_pc(void) {
    cpu_t cpu; bus_t bus; cartridge_t cartridge;
    setup_cpu_with_opcode(&cpu, &bus, &cartridge, 0x00);
    cpu_step(&cpu, &bus);
    TEST_ASSERT_EQUAL_UINT16(0x0001, cpu.pc);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cpu_init_sets_registers_to_initial_values);
    RUN_TEST(test_cpu_step_returns_minus1_on_unknown_opcode);
    RUN_TEST(test_cpu_step_executes_nop_correctly);
    RUN_TEST(test_cpu_step_nop_increments_pc);

    return UNITY_END();
}
