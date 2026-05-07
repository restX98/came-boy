#include "unity.h"
#include "log_helpers.h"

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
    TEST_ASSERT_FALSE(cpu.ime.enabled);
    TEST_ASSERT_FALSE(cpu.ime.scheduled);
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

// ---- ime scheduling ----

void test_cpu_step_promotes_ime_scheduled_to_enabled_before_execution(void) {
    uint8_t opcode = 0x00;
    bus_read_stats.calls[0].return_value = opcode;

    int mock_operation(cpu_t * _cpu, bus_t * _bus, uint8_t _opcode) {
        (void)_bus; (void)_opcode; (void)cpu;
        return 4;
    }
    opcode_table[opcode] = mock_operation;

    cpu.ime.scheduled = true;
    cpu.ime.enabled = false;

    cpu_step(&cpu, &bus);

    TEST_ASSERT_TRUE(cpu.ime.enabled);
    TEST_ASSERT_FALSE(cpu.ime.scheduled);
}

void test_cpu_step_wait_until_next_step_to_set_ime(void) {
    uint8_t op_nop = 0x00;
    uint8_t op_schedule = 0x01;
    bus_read_stats.calls[0].return_value = op_schedule;
    bus_read_stats.calls[1].return_value = op_nop;

    int mock_operation(cpu_t * _cpu, bus_t * _bus, uint8_t _opcode) {
        (void)_cpu; (void)_bus;
        if (_opcode == op_schedule) {
            cpu.ime.scheduled = true;
        }
        return 1;
    }
    opcode_table[op_nop] = mock_operation;
    opcode_table[op_schedule] = mock_operation;

    cpu.ime.scheduled = false;
    cpu.ime.enabled = false;

    cpu_step(&cpu, &bus);
    cpu_step(&cpu, &bus);

    TEST_ASSERT_TRUE(cpu.ime.enabled);
    TEST_ASSERT_FALSE(cpu.ime.scheduled);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cpu_init_sets_registers_to_initial_values);
    RUN_TEST(test_cpu_step_returns_minus1_on_unknown_opcode);
    RUN_TEST(test_cpu_step_returns_cycles_from_opcode);
    RUN_TEST(test_cpu_step_increments_pc_by_1_before_execution);
    RUN_TEST(test_cpu_step_promotes_ime_scheduled_to_enabled_before_execution);
    RUN_TEST(test_cpu_step_wait_until_next_step_to_set_ime);

    return UNITY_END();
}
