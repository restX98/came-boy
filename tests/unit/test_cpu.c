#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "cpu.h"
#include "isa/opcodes.h"

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

typedef struct {
    bus_t *bus;
    uint16_t addr;
    uint8_t value;
} bus_write_call_t;

typedef struct {
    size_t call_count;
    bus_write_call_t calls[10];
} bus_write_stats_t;

static bus_write_stats_t bus_write_stats;

void bus_write(bus_t *bus, uint16_t addr, uint8_t value) {
    if (bus_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for bus_write_stats");
    }

    bus_write_call_t *call = &bus_write_stats.calls[bus_write_stats.call_count];
    call->bus = bus;
    call->addr = addr;
    call->value = value;

    bus_write_stats.call_count++;
}

typedef struct {
    const interrupt_regs_t *interrupts;
    int return_value;
} interrupts_pending_call_t;

typedef struct {
    size_t call_count;
    interrupts_pending_call_t calls[10];
} interrupts_pending_stats_t;

static interrupts_pending_stats_t interrupts_pending_stats;

int interrupts_pending(const interrupt_regs_t *interrupts) {
    if (interrupts_pending_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_pending_stats");
    }

    interrupts_pending_call_t *call = &interrupts_pending_stats.calls[interrupts_pending_stats.call_count];
    call->interrupts = interrupts;

    interrupts_pending_stats.call_count++;

    return call->return_value;
}

typedef struct {
    interrupt_regs_t *interrupts;
    interrupt_t interrupt;
} interrupts_acknowledge_call_t;

typedef struct {
    size_t call_count;
    interrupts_acknowledge_call_t calls[10];
} interrupts_acknowledge_stats_t;

static interrupts_acknowledge_stats_t interrupts_acknowledge_stats;

void interrupts_acknowledge(interrupt_regs_t *interrupts, interrupt_t interrupt) {
    if (interrupts_acknowledge_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_acknowledge_stats");
    }

    interrupts_acknowledge_call_t *call = &interrupts_acknowledge_stats.calls[interrupts_acknowledge_stats.call_count];
    call->interrupts = interrupts;
    call->interrupt = interrupt;

    interrupts_acknowledge_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    cpu = (cpu_t){ 0 };
    bus = (bus_t){ 0 };
    memset(opcode_table, 0, sizeof(opcode_table));

    bus_read_stats = (bus_read_stats_t){ 0 };
    bus_write_stats = (bus_write_stats_t){ 0 };
    interrupts_pending_stats = (interrupts_pending_stats_t){ 0 };
    interrupts_acknowledge_stats = (interrupts_acknowledge_stats_t){ 0 };

    for (size_t i = 0; i < 10; i++) {
        interrupts_pending_stats.calls[i].return_value = -1;
    }
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
        (void)_cpu; (void)_bus; (void)_opcode;
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

// ---- halt ----

void test_cpu_step_returns_4_and_stays_halted_when_no_pending_interrupt(void) {
    cpu.halted = true;

    int cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(4, cycles);
    TEST_ASSERT_TRUE(cpu.halted);
    TEST_ASSERT_EQUAL_size_t(0, bus_read_stats.call_count);
}

void test_cpu_step_wakes_from_halt_when_pending_interrupt_and_ime_disabled(void) {
    cpu.halted = true;
    cpu.ime.enabled = false;
    interrupts_pending_stats.calls[0].return_value = 0;

    int cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(4, cycles);
    TEST_ASSERT_FALSE(cpu.halted);
    TEST_ASSERT_EQUAL_size_t(0, bus_read_stats.call_count);
}

// ---- halt bug ----

void test_cpu_step_does_not_increment_pc_when_halt_bug_is_set(void) {
    uint8_t opcode = 0x00;
    bus_read_stats.calls[0].return_value = opcode;

    int mock_operation(cpu_t * _cpu, bus_t * _bus, uint8_t _opcode) {
        (void)_cpu; (void)_bus; (void)_opcode;
        return 1;
    }
    opcode_table[opcode] = mock_operation;

    cpu.halt_bug = true;
    cpu.pc = 0x0100;

    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(0x0100, cpu.pc);
    TEST_ASSERT_FALSE(cpu.halt_bug);
}

// ---- interrupt service routine ----

void test_cpu_step_services_interrupt_when_pending_and_ime_enabled(void) {
    cpu.halted = true;
    cpu.ime.enabled = true;
    interrupts_pending_stats.calls[0].return_value = 0;

    int cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(20, cycles);
    TEST_ASSERT_FALSE(cpu.halted);
    TEST_ASSERT_FALSE(cpu.ime.enabled);
}

void test_cpu_step_does_not_service_interrupt_when_ime_disabled(void) {
    uint8_t opcode = 0x00;
    bus_read_stats.calls[0].return_value = opcode;

    int mock_operation(cpu_t * _cpu, bus_t * _bus, uint8_t _opcode) {
        (void)_cpu; (void)_bus; (void)_opcode;
        return 4;
    }
    opcode_table[opcode] = mock_operation;

    cpu.ime.enabled = false;
    interrupts_pending_stats.calls[0].return_value = 0;

    int cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(4, cycles);
    TEST_ASSERT_EQUAL_size_t(0, interrupts_acknowledge_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(0, bus_write_stats.call_count);
}

void test_cpu_step_acknowledges_pending_interrupt(void) {
    cpu.ime.enabled = true;
    interrupts_pending_stats.calls[0].return_value = 2;
    interrupts_pending_stats.calls[1].return_value = 2; // re-check after high-byte push

    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_acknowledge_stats.call_count);
    TEST_ASSERT_EQUAL_INT(2, interrupts_acknowledge_stats.calls[0].interrupt);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg.interrupts, interrupts_acknowledge_stats.calls[0].interrupts);
}

void test_cpu_step_pushes_pc_to_stack_when_servicing_interrupt(void) {
    cpu.ime.enabled = true;
    cpu.sp = 0xFFFE;
    cpu.pc = 0x1234;
    interrupts_pending_stats.calls[0].return_value = 0;
    interrupts_pending_stats.calls[1].return_value = 0;

    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_size_t(2, bus_write_stats.call_count);
    TEST_ASSERT_EQUAL_UINT16(0xFFFD, bus_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x12, bus_write_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, bus_write_stats.calls[1].addr);
    TEST_ASSERT_EQUAL_UINT8(0x34, bus_write_stats.calls[1].value);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, cpu.sp);
}

void test_cpu_step_jumps_to_handler_address_for_pending_interrupt(void) {
    const uint16_t expected_handlers[] = { 0x40, 0x48, 0x50, 0x58, 0x60 };

    for (size_t i = 0; i < 5; i++) {
        setUp();
        cpu.ime.enabled = true;
        cpu.sp = 0xFFFE;
        interrupts_pending_stats.calls[0].return_value = (int)i;
        interrupts_pending_stats.calls[1].return_value = (int)i;

        cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT16(expected_handlers[i], cpu.pc);
    }
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cpu_init_sets_registers_to_initial_values);
    RUN_TEST(test_cpu_step_returns_minus1_on_unknown_opcode);
    RUN_TEST(test_cpu_step_returns_cycles_from_opcode);
    RUN_TEST(test_cpu_step_increments_pc_by_1_before_execution);
    RUN_TEST(test_cpu_step_promotes_ime_scheduled_to_enabled_before_execution);
    RUN_TEST(test_cpu_step_wait_until_next_step_to_set_ime);
    RUN_TEST(test_cpu_step_returns_4_and_stays_halted_when_no_pending_interrupt);
    RUN_TEST(test_cpu_step_wakes_from_halt_when_pending_interrupt_and_ime_disabled);
    RUN_TEST(test_cpu_step_does_not_increment_pc_when_halt_bug_is_set);
    RUN_TEST(test_cpu_step_services_interrupt_when_pending_and_ime_enabled);
    RUN_TEST(test_cpu_step_does_not_service_interrupt_when_ime_disabled);
    RUN_TEST(test_cpu_step_acknowledges_pending_interrupt);
    RUN_TEST(test_cpu_step_pushes_pc_to_stack_when_servicing_interrupt);
    RUN_TEST(test_cpu_step_jumps_to_handler_address_for_pending_interrupt);

    return UNITY_END();
}
