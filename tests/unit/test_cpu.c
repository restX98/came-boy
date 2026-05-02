#include "unity.h"
#include "test_helpers.h"

#include <assert.h>
#include <string.h>

#include "cpu.h"
#include "opcodes.h"

static cpu_t cpu;
static bus_t bus;
opcode_fn opcode_table[256];

// ---- Mock functions ----

typedef struct {
    bus_t *bus;
    uint16_t addr;
    uint8_t return_value;
} bus_read_call_t;

typedef struct {
    size_t call_count;
    bus_read_call_t calls[10];
} bus_read_stats_t;

static bus_read_stats_t bus_read_stats;

uint8_t bus_read(bus_t *bus, uint16_t addr) {
    if (bus_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for bus_read_stats");
    }

    bus_read_call_t *call = &bus_read_stats.calls[bus_read_stats.call_count];
    call->bus = bus;
    call->addr = addr;

    bus_read_stats.call_count++;

    return bus_read_stats.calls[bus_read_stats.call_count - 1].return_value;

}

void setUp(void) {
    suppress_logs();

    cpu = (cpu_t){ 0 };
    bus = (bus_t){ 0 };
    memset(opcode_table, 0, sizeof(opcode_table));

    bus_read_stats = (bus_read_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- cpu_init ----

void test_cpu_init_sets_registers_to_initial_values(void) {
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
    bus_read_stats.calls[0].return_value = 0xD3;

    TEST_ASSERT_EQUAL_INT(-1, cpu_step(&cpu, &bus));
}

void test_cpu_step_returns_cycles_from_opcode(void) {
    uint8_t opcode = 0x00;
    bus_read_stats.calls[0].return_value = opcode;

    int expected_cycles = 10;
    int mock_operation(cpu_t * _cpu, bus_t * _bus, uint8_t _opcode) {
        (void)_cpu; (void)_bus; (void)_opcode;
        return expected_cycles;
    }
    opcode_table[opcode] = &mock_operation;

    int actual_cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(expected_cycles, actual_cycles);
}

void test_cpu_step_increments_pc_by_1_before_execution(void) {
    uint8_t opcode = 0x00;
    bus_read_stats.calls[0].return_value = opcode;

    int mock_operation(cpu_t * _cpu, bus_t * _bus, uint8_t _opcode) {
        (void)_cpu; (void)_bus; (void)_opcode;
        return 1;
    }
    opcode_table[opcode] = mock_operation;

    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(1, cpu.pc);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cpu_init_sets_registers_to_initial_values);
    RUN_TEST(test_cpu_step_returns_minus1_on_unknown_opcode);
    RUN_TEST(test_cpu_step_returns_cycles_from_opcode);
    RUN_TEST(test_cpu_step_increments_pc_by_1_before_execution);

    return UNITY_END();
}
