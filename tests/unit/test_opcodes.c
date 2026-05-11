#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "alu.h"
#include "opcodes.h"

static cpu_t mock_cpu = { 0 };
static bus_t mock_bus = { 0 };
static uint8_t mock_memory[0xFFFF] = { 0 };

// ---- Mock functions ----

// Mock alu_add8
typedef struct {
    uint8_t a;
    uint8_t value;
    uint8_t carry;
    alu8_result_t return_value;
} alu_add8_call_t;

typedef struct {
    size_t call_count;
    alu_add8_call_t calls[10];
} alu_add8_stats_t;

static alu_add8_stats_t alu_add8_stats;

alu8_result_t alu_add8(uint8_t a, uint8_t value, uint8_t carry) {
    if (alu_add8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_add8");
    }

    alu_add8_call_t *call = &alu_add8_stats.calls[alu_add8_stats.call_count];
    call->a = a;
    call->value = value;
    call->carry = carry;

    alu_add8_stats.call_count++;

    return alu_add8_stats.calls[alu_add8_stats.call_count - 1].return_value;
}

// Mock alu_sub8
typedef struct {
    uint8_t a;
    uint8_t value;
    uint8_t carry;
    alu8_result_t return_value;
} alu_sub8_call_t;

typedef struct {
    size_t call_count;
    alu_sub8_call_t calls[10];
} alu_sub8_stats_t;

static alu_sub8_stats_t alu_sub8_stats;

alu8_result_t alu_sub8(uint8_t a, uint8_t value, uint8_t carry) {
    if (alu_sub8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_sub8");
    }

    alu_sub8_call_t *call = &alu_sub8_stats.calls[alu_sub8_stats.call_count];
    call->a = a;
    call->value = value;
    call->carry = carry;

    alu_sub8_stats.call_count++;

    return alu_sub8_stats.calls[alu_sub8_stats.call_count - 1].return_value;
}

// Mock alu_inc8
typedef struct {
    uint8_t value;
    alu8_result_t return_value;
} alu_inc8_call_t;

typedef struct {
    size_t call_count;
    alu_inc8_call_t calls[10];
} alu_inc8_stats_t;

static alu_inc8_stats_t alu_inc8_stats;

alu8_result_t alu_inc8(uint8_t value) {
    if (alu_inc8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_inc8");
    }

    alu_inc8_call_t *call = &alu_inc8_stats.calls[alu_inc8_stats.call_count];
    call->value = value;

    alu_inc8_stats.call_count++;

    return alu_inc8_stats.calls[alu_inc8_stats.call_count - 1].return_value;
}

// Mock alu_dec8
typedef struct {
    uint8_t value;
    alu8_result_t return_value;
} alu_dec8_call_t;

typedef struct {
    size_t call_count;
    alu_dec8_call_t calls[10];
} alu_dec8_stats_t;

static alu_dec8_stats_t alu_dec8_stats;

alu8_result_t alu_dec8(uint8_t value) {
    if (alu_dec8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_dec8");
    }

    alu_dec8_call_t *call = &alu_dec8_stats.calls[alu_dec8_stats.call_count];
    call->value = value;

    alu_dec8_stats.call_count++;

    return alu_dec8_stats.calls[alu_dec8_stats.call_count - 1].return_value;
}

// Mock alu_and8
typedef struct {
    uint8_t a;
    uint8_t value;
    alu8_result_t return_value;
} alu_and8_call_t;

typedef struct {
    size_t call_count;
    alu_and8_call_t calls[10];
} alu_and8_stats_t;

static alu_and8_stats_t alu_and8_stats;

alu8_result_t alu_and8(uint8_t a, uint8_t value) {
    if (alu_and8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_and8");
    }

    alu_and8_call_t *call = &alu_and8_stats.calls[alu_and8_stats.call_count];
    call->a = a;
    call->value = value;

    alu_and8_stats.call_count++;

    return alu_and8_stats.calls[alu_and8_stats.call_count - 1].return_value;
}

// Mock alu_or8
typedef struct {
    uint8_t a;
    uint8_t value;
    alu8_result_t return_value;
} alu_or8_call_t;

typedef struct {
    size_t call_count;
    alu_or8_call_t calls[10];
} alu_or8_stats_t;

static alu_or8_stats_t alu_or8_stats;

alu8_result_t alu_or8(uint8_t a, uint8_t value) {
    if (alu_or8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_or8");
    }

    alu_or8_call_t *call = &alu_or8_stats.calls[alu_or8_stats.call_count];
    call->a = a;
    call->value = value;

    alu_or8_stats.call_count++;

    return alu_or8_stats.calls[alu_or8_stats.call_count - 1].return_value;
}

// Mock alu_xor8
typedef struct {
    uint8_t a;
    uint8_t value;
    alu8_result_t return_value;
} alu_xor8_call_t;

typedef struct {
    size_t call_count;
    alu_xor8_call_t calls[10];
} alu_xor8_stats_t;

static alu_xor8_stats_t alu_xor8_stats;

alu8_result_t alu_xor8(uint8_t a, uint8_t value) {
    if (alu_xor8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_xor8");
    }

    alu_xor8_call_t *call = &alu_xor8_stats.calls[alu_xor8_stats.call_count];
    call->a = a;
    call->value = value;

    alu_xor8_stats.call_count++;

    return alu_xor8_stats.calls[alu_xor8_stats.call_count - 1].return_value;
}

// Mock alu_add16
typedef struct {
    uint16_t hl;
    uint16_t value;
    alu16_result_t return_value;
} alu_add16_call_t;

typedef struct {
    size_t call_count;
    alu_add16_call_t calls[10];
} alu_add16_stats_t;

static alu_add16_stats_t alu_add16_stats;

alu16_result_t alu_add16(uint16_t hl, uint16_t value) {
    if (alu_add16_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_add16");
    }

    alu_add16_call_t *call = &alu_add16_stats.calls[alu_add16_stats.call_count];
    call->hl = hl;
    call->value = value;

    alu_add16_stats.call_count++;

    return alu_add16_stats.calls[alu_add16_stats.call_count - 1].return_value;
}

// Mock alu_add16_s8
typedef struct {
    uint16_t base;
    int8_t offset;
    alu16_result_t return_value;
} alu_add16_s8_call_t;

typedef struct {
    size_t call_count;
    alu_add16_s8_call_t calls[10];
} alu_add16_s8_stats_t;

static alu_add16_s8_stats_t alu_add16_s8_stats;

alu16_result_t alu_add16_s8(uint16_t base, int8_t offset) {
    if (alu_add16_s8_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for alu_add16_s8");
    }

    alu_add16_s8_call_t *call = &alu_add16_s8_stats.calls[alu_add16_s8_stats.call_count];
    call->base = base;
    call->offset = offset;

    alu_add16_s8_stats.call_count++;

    return alu_add16_s8_stats.calls[alu_add16_s8_stats.call_count - 1].return_value;
}

void setUp(void) {
    suppress_logs();
    alu_add8_stats = (alu_add8_stats_t){ 0 };
    alu_sub8_stats = (alu_sub8_stats_t){ 0 };
    alu_inc8_stats = (alu_inc8_stats_t){ 0 };
    alu_dec8_stats = (alu_dec8_stats_t){ 0 };
    alu_and8_stats = (alu_and8_stats_t){ 0 };
    alu_or8_stats = (alu_or8_stats_t){ 0 };
    alu_xor8_stats = (alu_xor8_stats_t){ 0 };
    alu_add16_stats = (alu_add16_stats_t){ 0 };
    alu_add16_s8_stats = (alu_add16_s8_stats_t){ 0 };
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

void test_op_add_hl_r16_all_registers(void) {
    struct {
        uint8_t opcode;
        uint16_t *reg;
    } cases[] = {
        {0x09, &mock_cpu.bc.reg}, // BC
        {0x19, &mock_cpu.de.reg}, // DE
        {0x39, &mock_cpu.sp}, // SP
    };

    for (int i = 0; i < 3; i++) {
        mock_cpu.hl.reg = 0x0003;
        *cases[i].reg = 0x0006;

        alu_add16_stats.calls[i].return_value = (alu16_result_t){
            .value = 0x0009,
            .status = {
                .half_carry = false,
                .carry = false,
            }
        };
        uint8_t opcode = cases[i].opcode;

        int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

        TEST_ASSERT_EQUAL(8, cycles);
        TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0x0009, mock_cpu.hl.reg);

        TEST_ASSERT_EQUAL_INT(i + 1, alu_add16_stats.call_count);
        TEST_ASSERT_EQUAL_UINT16(0x0003, alu_add16_stats.calls[i].hl);
        TEST_ASSERT_EQUAL_UINT16(0x0006, alu_add16_stats.calls[i].value);
    }
}

void test_op_add_hl_hl(void) {
    mock_cpu.hl.reg = 0x0003;

    alu_add16_stats.calls[0].return_value = (alu16_result_t){
            .value = 0x0006,
            .status = {
                .half_carry = false,
                .carry = false,
            }
    };

    uint8_t opcode = 0x29; // ADD HL, HL

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0x0006, mock_cpu.hl.reg);

    TEST_ASSERT_EQUAL_INT(1, alu_add16_stats.call_count);
    TEST_ASSERT_EQUAL_UINT16(0x0003, alu_add16_stats.calls[0].hl);
    TEST_ASSERT_EQUAL_UINT16(0x0003, alu_add16_stats.calls[0].value);
}

void test_op_add_hl_r16_reset_nhc_flags_if_no_overflow(void) {
    // 0000000000000101+
    // 0000000000000011=
    // 0001000000001000
    mock_cpu.hl.reg = 0x0005;
    mock_cpu.bc.reg = 0x0003;
    flag_set(&mock_cpu, FLAG_N); // N set beforehand

    alu_add16_stats.calls[0].return_value = (alu16_result_t){
            .value = 0x0008,
            .status = {
                .half_carry = false,
                .carry = false,
            }
    };

    uint8_t opcode = 0x09; // ADD HL, BC

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x0008, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT16(0x0005, alu_add16_stats.calls[0].hl);
    TEST_ASSERT_EQUAL_UINT16(0x0003, alu_add16_stats.calls[0].value);
}

void test_op_add_hl_r16_set_half_carry_if_overflow_from_bit_11(void) {
    // 0000111111111111+
    // 0000000000000001=
    // 0001000000000000
    mock_cpu.hl.reg = 0x0FFF;
    mock_cpu.bc.reg = 0x0001;

    alu_add16_stats.calls[0].return_value = (alu16_result_t){
            .value = 0x1000,
            .status = {
                .half_carry = true,
                .carry = false,
            }
    };

    uint8_t opcode = 0x09; // ADD HL, BC

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x1000, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT16(0x0FFF, alu_add16_stats.calls[0].hl);
    TEST_ASSERT_EQUAL_UINT16(0x0001, alu_add16_stats.calls[0].value);
}

void test_op_add_hl_r16_set_carry_if_overflow_from_bit_15(void) {
    // 1111111111111111+
    // 0000000000000001=
    // 0000000000000000
    mock_cpu.hl.reg = 0xFFFF;
    mock_cpu.bc.reg = 0x0001;

    alu_add16_stats.calls[0].return_value = (alu16_result_t){
            .value = 0x0000,
            .status = {
                .half_carry = true,
                .carry = true,
            }
    };

    uint8_t opcode = 0x09; // ADD HL, BC

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x0000, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));

    TEST_ASSERT_EQUAL_UINT16(0xFFFF, alu_add16_stats.calls[0].hl);
    TEST_ASSERT_EQUAL_UINT16(0x0001, alu_add16_stats.calls[0].value);
}

// ---- op_inc_r8 ----
void test_op_inc_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0x04, &mock_cpu.bc.hi}, // B
        {0x0C, &mock_cpu.bc.lo}, // C
        {0x14, &mock_cpu.de.hi}, // D
        {0x1C, &mock_cpu.de.lo}, // E
        {0x24, &mock_cpu.hl.hi}, // H
        {0x2C, &mock_cpu.hl.lo}, // L
        {0x3C, &mock_cpu.af.hi}, // A
    };

    for (int i = 0; i < 7; i++) {
        *cases[i].reg = 0x2;

        alu_inc8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x03,
            .status = {
                .zero = false,
                .half_carry = false,
            }
        };

        int cycles = opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL(4, cycles);
        TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(0x03, *cases[i].reg);

        TEST_ASSERT_EQUAL_INT(i + 1, alu_inc8_stats.call_count);
        TEST_ASSERT_EQUAL_UINT8(0x2, alu_inc8_stats.calls[i].value);
    }
}

void test_op_inc_hl_mem(void) {
    mock_cpu.hl.reg = 0x0;
    mock_memory[mock_cpu.hl.reg] = 0x2;

    alu_inc8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x03,
        .status = {
            .zero = false,
            .half_carry = false,
        }
    };

    uint8_t opcode = 0x34; // INC [HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x3, mock_memory[mock_cpu.hl.reg]);

    TEST_ASSERT_EQUAL_UINT8(0x2, alu_inc8_stats.calls[0].value);
}

void test_op_inc_r8_sets_z_flag_on_overflow(void) {
    mock_cpu.bc.hi = 0xFF;

    alu_inc8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x00,
       .status = {
           .zero = true,
           .half_carry = true,
       }
    };

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));

    TEST_ASSERT_EQUAL_UINT8(0xFF, alu_inc8_stats.calls[0].value);
}

void test_op_inc_r8_sets_h_flag_on_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x0F; // 00001111 -> 00010000

    alu_inc8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x10,
       .status = {
           .zero = false,
           .half_carry = true,
       }
    };

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x10, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));

    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_inc8_stats.calls[0].value);
}

void test_op_inc_r8_does_not_set_h_flag_when_no_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x10; // 00010000 -> 00010001

    alu_inc8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x11,
       .status = {
           .zero = false,
           .half_carry = false,
       }
    };

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x11, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(false, flag_get(&mock_cpu, FLAG_H));

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_inc8_stats.calls[0].value);
}

void test_op_inc_r8_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N); // N set beforehand
    mock_cpu.bc.hi = 0x01;

    alu_inc8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x02,
       .status = {
           .zero = false,
           .half_carry = false,
       }
    };

    uint8_t opcode = 0x04; // INC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));

    TEST_ASSERT_EQUAL_UINT8(0x01, alu_inc8_stats.calls[0].value);
}

// ---- op_dec_r8 ----
void test_op_dec_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0x05, &mock_cpu.bc.hi}, // B
        {0x0D, &mock_cpu.bc.lo}, // C
        {0x15, &mock_cpu.de.hi}, // D
        {0x1D, &mock_cpu.de.lo}, // E
        {0x25, &mock_cpu.hl.hi}, // H
        {0x2D, &mock_cpu.hl.lo}, // L
        {0x3D, &mock_cpu.af.hi}, // A
    };

    for (int i = 0; i < 7; i++) {
        *cases[i].reg = 0x2;

        alu_dec8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x01,
            .status = {
                .zero = false,
                .half_carry = false,
            }
        };

        int cycles = opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL(4, cycles);
        TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(0x1, *cases[i].reg);

        TEST_ASSERT_EQUAL_INT(i + 1, alu_dec8_stats.call_count);
        TEST_ASSERT_EQUAL_UINT8(0x2, alu_dec8_stats.calls[i].value);
    }
}

void test_op_dec_hl_mem(void) {
    mock_cpu.hl.reg = 0x0;
    mock_memory[mock_cpu.hl.reg] = 0x2;

    alu_dec8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x01,
        .status = {
            .zero = false,
            .half_carry = false,
        }
    };

    uint8_t opcode = 0x35; // DEC [HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x1, mock_memory[mock_cpu.hl.reg]);

    TEST_ASSERT_EQUAL_UINT8(0x2, alu_dec8_stats.calls[0].value);
}

void test_op_dec_r8_sets_z_flag_on_overflow(void) {
    mock_cpu.bc.hi = 0x01;

    alu_dec8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = false,
        }
    };

    uint8_t opcode = 0x05; // DEC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));

    TEST_ASSERT_EQUAL_UINT8(0x1, alu_dec8_stats.calls[0].value);
}

void test_op_dec_r8_sets_h_flag_on_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x10; // 00010000 -> 00001111

    alu_dec8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {
            .zero = false,
            .half_carry = true,
        }
    };

    uint8_t opcode = 0x05; // DEC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x0F, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_dec8_stats.calls[0].value);
}

void test_op_dec_r8_does_not_set_h_flag_when_no_nibble_overflow(void) {
    mock_cpu.bc.hi = 0x08; // 00001000 -> 00000111

    alu_dec8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x07,
        .status = {
            .zero = false,
            .half_carry = false,
        }
    };

    uint8_t opcode = 0x05; // DEC B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x07, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(false, flag_get(&mock_cpu, FLAG_H));

    TEST_ASSERT_EQUAL_UINT8(0x08, alu_dec8_stats.calls[0].value);
}

void test_op_dec_r8_wraps_and_sets_h(void) {
    mock_cpu.bc.hi = 0x00;

    alu_dec8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {
            .zero = false,
            .half_carry = true,
        }
    };


    uint8_t opcode = 0x05;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));

    TEST_ASSERT_EQUAL_UINT8(0x00, alu_dec8_stats.calls[0].value);
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

// ---- op_rlc_r8 ----
void test_op_rlc_r8_matrix(void) {
    // opcodes 0x00-0x05 = RLC B,C,D,E,H,L; 0x07 = RLC A (0x06 is [HL], tested separately)
    uint8_t r8_opcodes[7] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 };

    for (int i = 0; i < 7; i++) {
        *regs[i].reg = 0b10110100;

        int cycles = opcode_cb_table[r8_opcodes[i]](&mock_cpu, &mock_bus, r8_opcodes[i]);

        TEST_ASSERT_EQUAL_MESSAGE(8, cycles, "expected 8 cycles for register variant");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b01101001, *regs[i].reg, "expected left-rotation with msb wrapped to lsb");
    }
}

void test_op_rlc_r8_hl_mem(void) {
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0b10110100;

    uint8_t opcode = 0x06; // RLC [HL]

    int cycles = opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0b01101001, mock_memory[0x20]);
}

void test_op_rlc_r8_sets_carry_when_msb_is_1(void) {
    mock_cpu.bc.hi = 0b10000001; // B

    uint8_t opcode = 0x00; // RLC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b00000011, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rlc_r8_clears_carry_when_msb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b00000001; // B

    uint8_t opcode = 0x00; // RLC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b00000010, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rlc_r8_sets_zero_flag_when_result_is_zero(void) {
    mock_cpu.bc.hi = 0x00; // B = 0, rotation of 0 stays 0

    uint8_t opcode = 0x00; // RLC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rlc_r8_clears_zero_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.bc.hi = 0b01000000; // B

    uint8_t opcode = 0x00; // RLC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rlc_r8_clears_n_and_h_flags(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.bc.hi = 0b01000001; // B

    uint8_t opcode = 0x00; // RLC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_rrc_r8 ----
void test_op_rrc_r8_matrix(void) {
    // opcodes 0x08-0x0D = RRC B,C,D,E,H,L; 0x0F = RRC A (0x0E is [HL], tested separately)
    uint8_t r8_opcodes[7] = { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F };

    for (int i = 0; i < 7; i++) {
        *regs[i].reg = 0b10110101;

        int cycles = opcode_cb_table[r8_opcodes[i]](&mock_cpu, &mock_bus, r8_opcodes[i]);

        TEST_ASSERT_EQUAL_MESSAGE(8, cycles, "expected 8 cycles for register variant");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b11011010, *regs[i].reg, "expected right-rotation with lsb wrapped to msb");
    }
}

void test_op_rrc_r8_hl_mem(void) {
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0b10110101;

    uint8_t opcode = 0x0E; // RRC [HL]

    int cycles = opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0b11011010, mock_memory[0x20]);
}

void test_op_rrc_r8_sets_carry_when_lsb_is_1(void) {
    mock_cpu.bc.hi = 0b10000001; // B

    uint8_t opcode = 0x08; // RRC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b11000000, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rrc_r8_clears_carry_when_lsb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b10000000; // B

    uint8_t opcode = 0x08; // RRC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b01000000, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rrc_r8_sets_zero_flag_when_result_is_zero(void) {
    mock_cpu.bc.hi = 0x00; // B = 0, rotation of 0 stays 0

    uint8_t opcode = 0x08; // RRC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rrc_r8_clears_zero_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.bc.hi = 0b00000010; // B

    uint8_t opcode = 0x08; // RRC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rrc_r8_clears_n_and_h_flags(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.bc.hi = 0b10000010; // B

    uint8_t opcode = 0x08; // RRC B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_rl_r8 ----
void test_op_rl_r8_matrix(void) {

    // opcodes 0x10-0x15 = RL B,C,D,E,H,L; 0x17 = RL A (0x16 is [HL], tested separately)
    uint8_t r8_opcodes[7] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x17 };

    for (int i = 0; i < 7; i++) {
        flag_clear(&mock_cpu, FLAG_C);
        *regs[i].reg = 0b10110100; // carry_in = 0 (default)

        int cycles = opcode_cb_table[r8_opcodes[i]](&mock_cpu, &mock_bus, r8_opcodes[i]);

        TEST_ASSERT_EQUAL_MESSAGE(8, cycles, "expected 8 cycles for register variant");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b01101000, *regs[i].reg, "expected left-shift with carry_in=0 in bit 0");
    }
}

void test_op_rl_r8_hl_mem(void) {
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0b10110100;

    uint8_t opcode = 0x16; // RL [HL]

    int cycles = opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0b01101000, mock_memory[0x20]);
}

void test_op_rl_r8_inserts_carry_into_bit0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b00000000; // B

    uint8_t opcode = 0x10; // RL B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b00000001, mock_cpu.bc.hi);
}

void test_op_rl_r8_sets_carry_when_msb_is_1(void) {
    mock_cpu.bc.hi = 0b10000000; // B

    uint8_t opcode = 0x10; // RL B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rl_r8_clears_carry_when_msb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b01000000; // B, carry_in=1 will go into bit 0

    uint8_t opcode = 0x10; // RL B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rl_r8_sets_zero_flag_when_result_is_zero(void) {
    mock_cpu.bc.hi = 0x00; // carry_in = 0, result = 0

    uint8_t opcode = 0x10; // RL B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rl_r8_clears_zero_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.bc.hi = 0b01000000; // B

    uint8_t opcode = 0x10; // RL B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rl_r8_clears_n_and_h_flags(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.bc.hi = 0b01000010; // B

    uint8_t opcode = 0x10; // RL B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_rr_r8 ----
void test_op_rr_r8_matrix(void) {
    // opcodes 0x18-0x1D = RR B,C,D,E,H,L; 0x1F = RR A (0x1E is [HL], tested separately)
    uint8_t r8_opcodes[7] = { 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1F };

    for (int i = 0; i < 7; i++) {
        flag_clear(&mock_cpu, FLAG_C);
        *regs[i].reg = 0b10110101; // carry_in = 0 (default)

        int cycles = opcode_cb_table[r8_opcodes[i]](&mock_cpu, &mock_bus, r8_opcodes[i]);

        TEST_ASSERT_EQUAL_MESSAGE(8, cycles, "expected 8 cycles for register variant");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b01011010, *regs[i].reg, "expected right-shift with carry_in=0 in bit 7");
    }
}

void test_op_rr_r8_hl_mem(void) {
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0b10110101;

    uint8_t opcode = 0x1E; // RR [HL]

    int cycles = opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0b01011010, mock_memory[0x20]);
}

void test_op_rr_r8_inserts_carry_into_bit7(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b00000000; // B

    uint8_t opcode = 0x18; // RR B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b10000000, mock_cpu.bc.hi);
}

void test_op_rr_r8_sets_carry_when_lsb_is_1(void) {
    mock_cpu.bc.hi = 0b00000001; // B

    uint8_t opcode = 0x18; // RR B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rr_r8_clears_carry_when_lsb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b00000010; // B, carry_in=1 will go into bit 7

    uint8_t opcode = 0x18; // RR B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_rr_r8_sets_zero_flag_when_result_is_zero(void) {
    mock_cpu.bc.hi = 0x00; // carry_in = 0, result = 0

    uint8_t opcode = 0x18; // RR B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rr_r8_clears_zero_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.bc.hi = 0b00000010; // B

    uint8_t opcode = 0x18; // RR B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_rr_r8_clears_n_and_h_flags(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.bc.hi = 0b01000010; // B

    uint8_t opcode = 0x18; // RR B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_sla_r8 ----
void test_op_sla_r8_matrix(void) {
    // opcodes 0x20-0x25 = SLA B,C,D,E,H,L; 0x27 = SLA A (0x26 is [HL], tested separately)
    uint8_t r8_opcodes[7] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x27};

    for (int i = 0; i < 7; i++) {
        *regs[i].reg = 0b10110100;

        int cycles = opcode_cb_table[r8_opcodes[i]](&mock_cpu, &mock_bus, r8_opcodes[i]);

        TEST_ASSERT_EQUAL_MESSAGE(8, cycles, "expected 8 cycles for register variant");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b01101000, *regs[i].reg, "expected left-shift with 0 in bit 0");
    }
}

void test_op_sla_r8_hl_mem(void) {
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0b10110100;

    uint8_t opcode = 0x26; // SLA [HL]

    int cycles = opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0b01101000, mock_memory[0x20]);
}

void test_op_sla_r8_bit0_is_always_zero(void) {
    // SLA never wraps bit 7 into bit 0, unlike RLC
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b10000001; // B

    uint8_t opcode = 0x20; // SLA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b00000010, mock_cpu.bc.hi);
}

void test_op_sla_r8_sets_carry_when_msb_is_1(void) {
    mock_cpu.bc.hi = 0b10000000; // B

    uint8_t opcode = 0x20; // SLA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sla_r8_clears_carry_when_msb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b01000000; // B

    uint8_t opcode = 0x20; // SLA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sla_r8_sets_zero_flag_when_result_is_zero(void) {
    mock_cpu.bc.hi = 0b10000000; // B, result = 0x00

    uint8_t opcode = 0x20; // SLA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_sla_r8_clears_zero_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.bc.hi = 0b01000000; // B

    uint8_t opcode = 0x20; // SLA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_sla_r8_clears_n_and_h_flags(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.bc.hi = 0b01000010; // B

    uint8_t opcode = 0x20; // SLA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_sra_r8 ----
void test_op_sra_r8_matrix(void) {
    // opcodes 0x28-0x2D = SRA B,C,D,E,H,L; 0x2F = SRA A (0x2E is [HL], tested separately)
    uint8_t r8_opcodes[7] = {0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2F};

    for (int i = 0; i < 7; i++) {
        *regs[i].reg = 0b10110100;

        int cycles = opcode_cb_table[r8_opcodes[i]](&mock_cpu, &mock_bus, r8_opcodes[i]);

        TEST_ASSERT_EQUAL_MESSAGE(8, cycles, "expected 8 cycles for register variant");
        // bit 7 preserved (sign extension): 0b10110100 >> 1 | 0x80 = 0b11011010
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b11011010, *regs[i].reg, "expected right-shift with msb preserved");
    }
}

void test_op_sra_r8_hl_mem(void) {
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0b10110100;

    uint8_t opcode = 0x2E; // SRA [HL]

    int cycles = opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0b11011010, mock_memory[0x20]);
}

void test_op_sra_r8_preserves_msb(void) {
    // SRA preserves bit 7 (sign extension), unlike RR which fills from carry
    mock_cpu.bc.hi = 0b10000000; // B, MSB=1

    uint8_t opcode = 0x28; // SRA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0b11000000, mock_cpu.bc.hi);
}

void test_op_sra_r8_sets_carry_when_lsb_is_1(void) {
    mock_cpu.bc.hi = 0b00000001; // B

    uint8_t opcode = 0x28; // SRA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sra_r8_clears_carry_when_lsb_is_0(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0b10000010; // B

    uint8_t opcode = 0x28; // SRA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sra_r8_sets_zero_flag_when_result_is_zero(void) {
    mock_cpu.bc.hi = 0x00; // B = 0, result = 0

    uint8_t opcode = 0x28; // SRA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_sra_r8_clears_zero_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.bc.hi = 0b00000010; // B

    uint8_t opcode = 0x28; // SRA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_sra_r8_clears_n_and_h_flags(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    mock_cpu.bc.hi = 0b01000010; // B

    uint8_t opcode = 0x28; // SRA B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

// ---- op_swap_r8 ----
void test_op_swap_r8_matrix(void) {
    // opcodes 0x30-0x35 = SWAP B,C,D,E,H,L; 0x37 = SWAP A (0x36 is [HL], tested separately)
    uint8_t r8_opcodes[7] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x37};

    for (int i = 0; i < 7; i++) {
        *regs[i].reg = 0xAB;

        int cycles = opcode_cb_table[r8_opcodes[i]](&mock_cpu, &mock_bus, r8_opcodes[i]);

        TEST_ASSERT_EQUAL_MESSAGE(8, cycles, "expected 8 cycles for register variant");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0xBA, *regs[i].reg, "expected upper and lower nibbles swapped");
    }
}

void test_op_swap_r8_hl_mem(void) {
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0xAB;

    uint8_t opcode = 0x36; // SWAP [HL]

    int cycles = opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xBA, mock_memory[0x20]);
}

void test_op_swap_r8_sets_zero_flag_when_result_is_zero(void) {
    mock_cpu.bc.hi = 0x00; // B

    uint8_t opcode = 0x30; // SWAP B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.bc.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_swap_r8_clears_zero_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.bc.hi = 0xAB; // B

    uint8_t opcode = 0x30; // SWAP B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_swap_r8_always_clears_n_h_c_flags(void) {
    flag_set(&mock_cpu, FLAG_N);
    flag_set(&mock_cpu, FLAG_H);
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.bc.hi = 0xAB; // B

    uint8_t opcode = 0x30; // SWAP B

    opcode_cb_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
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

// ---- op_ret_cond ----
void test_op_ret_nz_condition_true(void) {
    mock_cpu.pc = 0x0100;
    mock_cpu.sp = 0xFFFC;
    flag_clear(&mock_cpu, FLAG_Z); // NZ = true

    mock_memory[0xFFFC] = 0x34; // lo byte of return address
    mock_memory[0xFFFC + 1] = 0x12; // hi byte of return address

    uint8_t opcode = 0xC0; // RET NZ

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(20, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

void test_op_ret_nz_condition_false(void) {
    mock_cpu.pc = 0x0100;
    mock_cpu.sp = 0xFFFC;
    flag_set(&mock_cpu, FLAG_Z); // NZ = false

    mock_memory[0xFFFC] = 0x34;
    mock_memory[0xFFFC + 1] = 0x12;

    uint8_t opcode = 0xC0; // RET NZ

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0100, mock_cpu.pc); // PC unchanged
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, mock_cpu.sp); // SP unchanged
}

void test_op_ret_z_condition_true(void) {
    mock_cpu.pc = 0x0200;
    mock_cpu.sp = 0xFFFA;
    flag_set(&mock_cpu, FLAG_Z); // Z = true

    mock_memory[0xFFFA] = 0xCD;
    mock_memory[0xFFFA + 1] = 0xAB;

    uint8_t opcode = 0xC8; // RET Z

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(20, cycles);
    TEST_ASSERT_EQUAL_UINT16(0xABCD, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, mock_cpu.sp);
}

void test_op_ret_z_condition_false(void) {
    mock_cpu.pc = 0x0200;
    mock_cpu.sp = 0xFFFA;
    flag_clear(&mock_cpu, FLAG_Z); // Z = false

    uint8_t opcode = 0xC8; // RET Z

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0200, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFA, mock_cpu.sp);
}

void test_op_ret_nc_condition_true(void) {
    mock_cpu.sp = 0xFFFC;
    flag_clear(&mock_cpu, FLAG_C); // NC = true

    mock_memory[0xFFFC] = 0x78;
    mock_memory[0xFFFC + 1] = 0x56;

    uint8_t opcode = 0xD0; // RET NC

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(20, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

void test_op_ret_c_condition_true(void) {
    mock_cpu.sp = 0xFFFC;
    flag_set(&mock_cpu, FLAG_C); // C = true

    mock_memory[0xFFFC] = 0x78;
    mock_memory[0xFFFC + 1] = 0x56;

    uint8_t opcode = 0xD8; // RET C

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(20, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

// ---- op_ret ----
void test_op_ret_loads_pc_from_stack(void) {
    mock_cpu.sp = 0xFFFC;
    mock_memory[0xFFFC] = 0x34; // lo byte
    mock_memory[0xFFFC + 1] = 0x12; // hi byte

    uint8_t opcode = 0xC9; // RET

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

void test_op_ret_cycles_independent_of_flags(void) {
    mock_cpu.sp = 0xFFFC;
    flag_set(&mock_cpu, FLAG_Z);
    flag_set(&mock_cpu, FLAG_C);

    uint8_t opcode = 0xC9; // RET

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
}

// ---- op_reti ----
void test_op_reti_loads_pc_from_stack(void) {
    mock_cpu.sp = 0xFFFC;
    mock_memory[0xFFFC] = 0x34; // lo byte
    mock_memory[0xFFFC + 1] = 0x12; // hi byte

    uint8_t opcode = 0xD9; // RETI

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

void test_op_reti_sets_ime_immediately(void) {
    mock_cpu.sp = 0xFFFC;
    mock_cpu.ime.enabled = false;

    uint8_t opcode = 0xD9; // RETI

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_TRUE(mock_cpu.ime.enabled);
}

void test_op_reti_sets_ime_even_when_not_scheduled(void) {
    mock_cpu.sp = 0xFFFC;
    mock_cpu.ime.enabled = false;
    mock_cpu.ime.scheduled = false;

    uint8_t opcode = 0xD9; // RETI

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // Unlike EI, RETI enables IME immediately without going through ime_scheduled
    TEST_ASSERT_TRUE(mock_cpu.ime.enabled);
}

// ---- op_jp_imm16 ----
void test_op_jp_imm16_jumps_to_address(void) {
    mock_cpu.pc = 0x0100;
    mock_memory[0x0100] = 0x34; // lo byte of target
    mock_memory[0x0101] = 0x12; // hi byte of target

    uint8_t opcode = 0xC3; // JP imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
}

void test_op_jp_imm16_high_address(void) {
    mock_cpu.pc = 0x0000;
    mock_memory[0x0000] = 0xFF; // lo byte
    mock_memory[0x0001] = 0x80; // hi byte — address above 0x7FFF

    uint8_t opcode = 0xC3; // JP imm16

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    // Must land at 0x80FF, not be misinterpreted as a negative signed value
    TEST_ASSERT_EQUAL_UINT16(0x80FF, mock_cpu.pc);
}

// ---- op_jp_cond_imm16 ----
void test_op_jp_nz_condition_true(void) {
    mock_cpu.pc = 0x0000;
    flag_clear(&mock_cpu, FLAG_Z); // NZ = true
    mock_memory[0x0000] = 0x34;
    mock_memory[0x0001] = 0x12;

    uint8_t opcode = 0xC2; // JP NZ,imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
}

void test_op_jp_nz_condition_false(void) {
    mock_cpu.pc = 0x0000;
    flag_set(&mock_cpu, FLAG_Z); // NZ = false
    mock_memory[0x0000] = 0x34;
    mock_memory[0x0001] = 0x12;

    uint8_t opcode = 0xC2; // JP NZ,imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0002, mock_cpu.pc); // only consumed imm16 bytes
}

void test_op_jp_z_condition_true(void) {
    mock_cpu.pc = 0x0000;
    flag_set(&mock_cpu, FLAG_Z);
    mock_memory[0x0000] = 0x78;
    mock_memory[0x0001] = 0x56;

    uint8_t opcode = 0xCA; // JP Z,imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
}

void test_op_jp_nc_condition_true(void) {
    mock_cpu.pc = 0x0000;
    flag_clear(&mock_cpu, FLAG_C);
    mock_memory[0x0000] = 0x78;
    mock_memory[0x0001] = 0x56;

    uint8_t opcode = 0xD2; // JP NC,imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
}

void test_op_jp_c_condition_true(void) {
    mock_cpu.pc = 0x0000;
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0x0000] = 0x78;
    mock_memory[0x0001] = 0x56;

    uint8_t opcode = 0xDA; // JP C,imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
}

void test_op_jp_cond_high_address(void) {
    mock_cpu.pc = 0x0000;
    flag_set(&mock_cpu, FLAG_Z);
    mock_memory[0x0000] = 0xFF;
    mock_memory[0x0001] = 0x80; // target = 0x80FF

    uint8_t opcode = 0xCA; // JP Z,imm16

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT16(0x80FF, mock_cpu.pc);
}

// ---- op_jp_hl ----
void test_op_jp_hl_jumps_to_hl(void) {
    mock_cpu.hl.reg = 0x1234;

    uint8_t opcode = 0xE9; // JP HL

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
}

void test_op_jp_hl_does_not_modify_hl(void) {
    mock_cpu.hl.reg = 0xABCD;

    opcode_table[0xE9](&mock_cpu, &mock_bus, 0xE9);

    TEST_ASSERT_EQUAL_UINT16(0xABCD, mock_cpu.hl.reg);
}

void test_op_jp_hl_high_address(void) {
    mock_cpu.hl.reg = 0xFF80; // top of HRAM

    opcode_table[0xE9](&mock_cpu, &mock_bus, 0xE9);

    TEST_ASSERT_EQUAL_UINT16(0xFF80, mock_cpu.pc);
}

// ---- op_call_imm16 ----
void test_op_call_imm16_jumps_to_address(void) {
    mock_cpu.pc = 0x0100;
    mock_cpu.sp = 0xFFFE;
    mock_memory[0x0100] = 0x34; // lo byte of target
    mock_memory[0x0101] = 0x12; // hi byte of target

    uint8_t opcode = 0xCD; // CALL imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(24, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
}

void test_op_call_imm16_pushes_return_address(void) {
    mock_cpu.pc = 0x0100;
    mock_cpu.sp = 0xFFFE;
    mock_memory[0x0100] = 0x34;
    mock_memory[0x0101] = 0x12;

    opcode_table[0xCD](&mock_cpu, &mock_bus, 0xCD);

    // Return address is 0x0102 (byte after the 3-byte CALL instruction)
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, mock_cpu.sp);
    TEST_ASSERT_EQUAL_UINT8(0x02, mock_memory[0xFFFC]); // lo byte at SP
    TEST_ASSERT_EQUAL_UINT8(0x01, mock_memory[0xFFFD]); // hi byte at SP+1
}

void test_op_call_imm16_return_address_roundtrip(void) {
    mock_cpu.pc = 0x0200;
    mock_cpu.sp = 0xFFFE;
    mock_memory[0x0200] = 0x00; // lo: jump to 0x0300
    mock_memory[0x0201] = 0x03; // hi

    opcode_table[0xCD](&mock_cpu, &mock_bus, 0xCD); // CALL 0x0300
    opcode_table[0xC9](&mock_cpu, &mock_bus, 0xC9); // RET

    TEST_ASSERT_EQUAL_UINT16(0x0202, mock_cpu.pc); // back to instruction after CALL
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp); // SP restored
}

// ---- op_call_cond_imm16 ----
void test_op_call_nz_condition_true(void) {
    mock_cpu.pc = 0x0100;
    mock_cpu.sp = 0xFFFE;
    flag_clear(&mock_cpu, FLAG_Z); // NZ = true
    mock_memory[0x0100] = 0x34;
    mock_memory[0x0101] = 0x12;

    uint8_t opcode = 0xC4; // CALL NZ,imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(24, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, mock_cpu.sp);
    TEST_ASSERT_EQUAL_UINT8(0x02, mock_memory[0xFFFC]); // lo of return addr
    TEST_ASSERT_EQUAL_UINT8(0x01, mock_memory[0xFFFD]); // hi of return addr
}

void test_op_call_nz_condition_false(void) {
    mock_cpu.pc = 0x0100;
    mock_cpu.sp = 0xFFFE;
    flag_set(&mock_cpu, FLAG_Z); // NZ = false
    mock_memory[0x0100] = 0x34;
    mock_memory[0x0101] = 0x12;

    uint8_t opcode = 0xC4; // CALL NZ,imm16

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0102, mock_cpu.pc); // only consumed imm16
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp); // SP unchanged
}

void test_op_call_z_condition_true(void) {
    mock_cpu.pc = 0x0000;
    mock_cpu.sp = 0xFFFE;
    flag_set(&mock_cpu, FLAG_Z);
    mock_memory[0x0000] = 0x78;
    mock_memory[0x0001] = 0x56;

    int cycles = opcode_table[0xCC](&mock_cpu, &mock_bus, 0xCC); // CALL Z,imm16

    TEST_ASSERT_EQUAL(24, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
}

void test_op_call_nc_condition_true(void) {
    mock_cpu.pc = 0x0000;
    mock_cpu.sp = 0xFFFE;
    flag_clear(&mock_cpu, FLAG_C);
    mock_memory[0x0000] = 0x78;
    mock_memory[0x0001] = 0x56;

    int cycles = opcode_table[0xD4](&mock_cpu, &mock_bus, 0xD4); // CALL NC,imm16

    TEST_ASSERT_EQUAL(24, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
}

void test_op_call_c_condition_true(void) {
    mock_cpu.pc = 0x0000;
    mock_cpu.sp = 0xFFFE;
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0x0000] = 0x78;
    mock_memory[0x0001] = 0x56;

    int cycles = opcode_table[0xDC](&mock_cpu, &mock_bus, 0xDC); // CALL C,imm16

    TEST_ASSERT_EQUAL(24, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, mock_cpu.pc);
}

// ---- op_rst_tgt3 ----
void test_op_rst_tgt3_matrix(void) {
    struct { uint8_t opcode; uint16_t vector; } cases[] = {
        { 0xC7, 0x0000 }, // RST $00
        { 0xCF, 0x0008 }, // RST $08
        { 0xD7, 0x0010 }, // RST $10
        { 0xDF, 0x0018 }, // RST $18
        { 0xE7, 0x0020 }, // RST $20
        { 0xEF, 0x0028 }, // RST $28
        { 0xF7, 0x0030 }, // RST $30
        { 0xFF, 0x0038 }, // RST $38
    };

    for (int i = 0; i < 8; i++) {
        mock_cpu.pc = 0x0150;
        mock_cpu.sp = 0xFFFE;
        memset(mock_memory, 0, sizeof(mock_memory));

        uint8_t opcode = cases[i].opcode;
        uint16_t expected_vector = cases[i].vector;

        int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

        TEST_ASSERT_EQUAL_MESSAGE(16, cycles, "cycle count");
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected_vector, mock_cpu.pc, "jump target");
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(0xFFFC, mock_cpu.sp, "SP after push");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x50, mock_memory[0xFFFC], "return addr lo");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x01, mock_memory[0xFFFD], "return addr hi");
    }
}

void test_op_rst_return_address_roundtrip(void) {
    mock_cpu.pc = 0x0150;
    mock_cpu.sp = 0xFFFE;

    opcode_table[0xC7](&mock_cpu, &mock_bus, 0xC7); // RST $00
    opcode_table[0xC9](&mock_cpu, &mock_bus, 0xC9); // RET

    TEST_ASSERT_EQUAL_UINT16(0x0150, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

// ---- op_pop_r16stk ----
void test_op_pop_r16stk_matrix(void) {
    struct {
        uint8_t opcode;
        uint16_t *reg;
        const char *name;
    } cases[] = {
        { 0xC1, &mock_cpu.bc.reg, "BC" },
        { 0xD1, &mock_cpu.de.reg, "DE" },
        { 0xE1, &mock_cpu.hl.reg, "HL" },
    };

    for (int i = 0; i < 3; i++) {
        mock_cpu.sp = 0xFFFC;
        mock_cpu.bc.reg = 0;
        mock_cpu.de.reg = 0;
        mock_cpu.hl.reg = 0;
        mock_memory[0xFFFC] = 0x34; // lo
        mock_memory[0xFFFC + 1] = 0x12; // hi

        int cycles = opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_MESSAGE(12, cycles, cases[i].name);
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(0x1234, *cases[i].reg, cases[i].name);
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(0xFFFE, mock_cpu.sp, cases[i].name);
    }
}

void test_op_pop_af_loads_flags(void) {
    // Stack contains F=0xF0 (Z=1, N=1, H=1, C=1) and A=0xAB
    mock_cpu.sp = 0xFFFC;
    mock_cpu.af.reg = 0;
    mock_memory[0xFFFC] = FLAG_Z | FLAG_N | FLAG_H | FLAG_C; // F byte (all flags set)
    mock_memory[0xFFFC + 1] = 0xAB;                          // A byte

    int cycles = opcode_table[0xF1](&mock_cpu, &mock_bus, 0xF1); // POP AF

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mock_cpu.af.hi);
    TEST_ASSERT_TRUE(flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_TRUE(flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_TRUE(flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_TRUE(flag_get(&mock_cpu, FLAG_C));
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

void test_op_pop_af_masks_lower_nibble_of_f(void) {
    // Stack F byte has lower nibble set — hardware always clears it
    mock_cpu.sp = 0xFFFC;
    mock_cpu.af.reg = 0;
    mock_memory[0xFFFC] = 0xFF; // F with lower nibble set (should be masked out)
    mock_memory[0xFFFC + 1] = 0x00;

    opcode_table[0xF1](&mock_cpu, &mock_bus, 0xF1); // POP AF

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.lo & 0x0F); // lower nibble of F is always 0
}

// ---- op_push_r16stk ----
void test_op_push_r16stk_matrix(void) {
    struct {
        uint8_t opcode;
        uint16_t value;
        uint16_t *reg;
        const char *name;
    } cases[] = {
        { 0xC5, 0x1234, &mock_cpu.bc.reg, "BC" },
        { 0xD5, 0x5678, &mock_cpu.de.reg, "DE" },
        { 0xE5, 0x9ABC, &mock_cpu.hl.reg, "HL" },
    };

    for (int i = 0; i < 3; i++) {
        mock_cpu.sp = 0xFFFE;
        memset(mock_memory, 0, sizeof(mock_memory));
        *cases[i].reg = cases[i].value;

        int cycles = opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_MESSAGE(16, cycles, cases[i].name);
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(0xFFFC, mock_cpu.sp, cases[i].name);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)(cases[i].value & 0xFF), mock_memory[0xFFFC], cases[i].name); // lo at SP
        TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)(cases[i].value >> 8), mock_memory[0xFFFD], cases[i].name); // hi at SP+1
    }
}

void test_op_push_af_stores_flags(void) {
    mock_cpu.sp = 0xFFFE;
    mock_cpu.af.hi = 0xAB; // A
    mock_cpu.af.lo = FLAG_Z | FLAG_N | FLAG_H | FLAG_C; // F = 0xF0

    int cycles = opcode_table[0xF5](&mock_cpu, &mock_bus, 0xF5); // PUSH AF

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0xFFFC, mock_cpu.sp);
    TEST_ASSERT_EQUAL_UINT8(FLAG_Z | FLAG_N | FLAG_H | FLAG_C, mock_memory[0xFFFC]); // F at lo
    TEST_ASSERT_EQUAL_UINT8(0xAB, mock_memory[0xFFFD]);                              // A at hi
}

void test_op_push_af_masks_lower_nibble_of_f(void) {
    mock_cpu.sp = 0xFFFE;
    mock_cpu.af.hi = 0x00;
    mock_cpu.af.lo = 0xFF; // F with lower nibble set (should be masked)

    opcode_table[0xF5](&mock_cpu, &mock_bus, 0xF5); // PUSH AF

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_memory[0xFFFC] & 0x0F); // lower nibble of pushed F is always 0
}

// ---- op_ldh_imm8mem_a ----
void test_op_ldh_imm8mem_a_writes_a_to_hram(void) {
    mock_cpu.af.hi = 0xAB;
    mock_memory[0x0000] = 0x10; // imm8 = 0x10 → address = 0xFF10

    int cycles = opcode_table[0xE0](&mock_cpu, &mock_bus, 0xE0);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mock_memory[0xFF10]);
}

void test_op_ldh_imm8mem_a_uses_ff00_base(void) {
    mock_cpu.af.hi = 0x55;
    mock_memory[0x0000] = 0x00; // imm8 = 0x00 → address = 0xFF00

    opcode_table[0xE0](&mock_cpu, &mock_bus, 0xE0);

    TEST_ASSERT_EQUAL_UINT8(0x55, mock_memory[0xFF00]);
}

// ---- op_ldh_c_mem_a ----
void test_op_ldh_c_mem_a_writes_a_to_hram(void) {
    mock_cpu.af.hi = 0xCD;
    mock_cpu.bc.lo = 0x20; // C = 0x20 → address = 0xFF20

    int cycles = opcode_table[0xE2](&mock_cpu, &mock_bus, 0xE2);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xCD, mock_memory[0xFF20]);
}

void test_op_ldh_c_mem_a_uses_ff00_base(void) {
    mock_cpu.af.hi = 0x77;
    mock_cpu.bc.lo = 0x00; // C = 0x00 → address = 0xFF00

    opcode_table[0xE2](&mock_cpu, &mock_bus, 0xE2);

    TEST_ASSERT_EQUAL_UINT8(0x77, mock_memory[0xFF00]);
}

// ---- op_ld_imm16mem_a ----
void test_op_ld_imm16mem_a_writes_a_to_address(void) {
    mock_cpu.af.hi = 0x42;
    mock_memory[0x0000] = 0x50; // lo
    mock_memory[0x0001] = 0x20; // hi → address = 0x2050

    int cycles = opcode_table[0xEA](&mock_cpu, &mock_bus, 0xEA);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x42, mock_memory[0x2050]);
}

// ---- op_ldh_a_imm8mem ----
void test_op_ldh_a_imm8mem_reads_from_hram(void) {
    mock_memory[0xFF30] = 0xBE;
    mock_memory[0x0000] = 0x30; // imm8 = 0x30 → address = 0xFF30

    int cycles = opcode_table[0xF0](&mock_cpu, &mock_bus, 0xF0);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xBE, mock_cpu.af.hi);
}

void test_op_ldh_a_imm8mem_uses_ff00_base(void) {
    mock_memory[0xFF00] = 0x77;
    mock_memory[0x0001] = 0x00; // imm8 = 0x00 → address = 0xFF00

    opcode_table[0xF0](&mock_cpu, &mock_bus, 0xF0);

    TEST_ASSERT_EQUAL_UINT8(0x77, mock_cpu.af.hi);
}

// ---- op_ldh_a_c_mem ----
void test_op_ldh_a_c_mem_reads_from_hram(void) {
    mock_cpu.bc.lo = 0x42; // C = 0x42 → address = 0xFF42
    mock_memory[0xFF42] = 0xDE;

    int cycles = opcode_table[0xF2](&mock_cpu, &mock_bus, 0xF2);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xDE, mock_cpu.af.hi);
}

void test_op_ldh_a_c_mem_uses_ff00_base(void) {
    mock_cpu.bc.lo = 0x00; // C = 0x00 → address = 0xFF00
    mock_memory[0xFF00] = 0x99;

    opcode_table[0xF2](&mock_cpu, &mock_bus, 0xF2);

    TEST_ASSERT_EQUAL_UINT8(0x99, mock_cpu.af.hi);
}

// ---- op_ld_a_imm16mem ----
void test_op_ld_a_imm16mem_reads_from_address(void) {
    mock_memory[0x0000] = 0x50; // lo
    mock_memory[0x0001] = 0x20; // hi → address = 0x2050
    mock_memory[0x2050] = 0xAB;

    int cycles = opcode_table[0xFA](&mock_cpu, &mock_bus, 0xFA);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mock_cpu.af.hi);
}

// ---- op_ld_r8_r8 ----

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

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x32,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };


    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x32, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_INT(1, alu_add8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x22, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry);
}

void test_op_add_a_r8_sets_zero_flag(void) {
    mock_cpu.af.hi = 0x80;
    mock_cpu.bc.hi = 0x80; // B

    uint8_t opcode = 0x80; // ADD A,B

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = false,
            .carry = true,
        }
    };

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));

    TEST_ASSERT_EQUAL_UINT8(0x80, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x80, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry);
}

void test_op_add_a_r8_sets_half_carry_and_carry(void) {
    mock_cpu.af.hi = 0xFF;
    mock_cpu.bc.hi = 0x01; // B

    uint8_t opcode = 0x80; // ADD A,B

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = true,
            .carry = true,
        }
    };

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xFF, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x01, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry);
}

void test_op_add_a_hl_mem(void) {
    mock_cpu.af.hi = 0x10;
    mock_cpu.hl.reg = 0x20;

    mock_memory[0x20] = 0x22;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x32,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x86; // ADD A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x32, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x22, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry);
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

        alu_add8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x15,
            .status = {
                .zero = false,
                .half_carry = false,
                .carry = false,
            }
        };

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x15, mock_cpu.af.hi);

        TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x05, alu_add8_stats.calls[i].value);
        TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[i].carry);
    }
}

void test_op_add_a_a(void) {
    mock_cpu.af.hi = 0x10;

    uint8_t opcode = 0x87; // ADD A,A

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry);
}

// ---- op_adc_a_r8 ----
void test_op_adc_a_r8_no_carry_in(void) {
    flag_clear(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x22; // B

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x32,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x88; // ADC A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x32, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x22, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry);
}

void test_op_adc_a_r8_with_carry_in(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x22; // B

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x33,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x33, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x22, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[0].carry);
}

void test_op_adc_a_r8_sets_zero_flag(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0xFF;
    mock_cpu.bc.hi = 0x00; // B

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = true,
            .carry = true,
        }
    };

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));

    TEST_ASSERT_EQUAL_UINT8(0xFF, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x00, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[0].carry);
}

void test_op_adc_a_r8_sets_half_carry(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x0F;
    mock_cpu.bc.hi = 0x00; // B

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x10,
        .status = {
            .zero = true,
            .half_carry = true,
            .carry = true,
        }
    };

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x10, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));

    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x00, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[0].carry);
}

void test_op_adc_a_r8_sets_carry(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0xF0;
    mock_cpu.bc.hi = 0x0F; // B

    uint8_t opcode = 0x88; // ADC A,B

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = true,
            .carry = true,
        }
    };

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[0].carry);
}

void test_op_adc_a_r8_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N);

    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x12,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x88; // ADC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x01, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry);
}

void test_op_adc_a_hl_mem(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;
    mock_cpu.hl.reg = 0x20;
    mock_memory[0x20] = 0x22;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x33,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x8E; // ADC A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x33, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x22, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[0].carry);
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

        alu_add8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x16,
            .status = {
                .zero = false,
                .half_carry = false,
                .carry = false,
            }
        };

        mock_cpu.af.hi = 0x10;
        *cases[i].reg = 0x05;

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x16, mock_cpu.af.hi);

        TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x05, alu_add8_stats.calls[i].value);
        TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[i].carry);
    }
}

void test_op_adc_a_a(void) {
    flag_set(&mock_cpu, FLAG_C);

    mock_cpu.af.hi = 0x10;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x21,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x8F; // ADC A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x21, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[0].carry);
}

// ---- op_sub_a_r8 ----
void test_op_sub_a_r8(void) {
    mock_cpu.af.hi = 0x30;
    mock_cpu.bc.hi = 0x10; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x90; // SUB A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_INT(1, alu_sub8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_sub_a_r8_sets_zero_flag(void) {
    mock_cpu.af.hi = 0x42;
    mock_cpu.bc.hi = 0x42; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x90; // SUB A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));

    TEST_ASSERT_EQUAL_UINT8(0x42, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x42, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_sub_a_r8_sets_half_borrow_and_carry(void) {
    mock_cpu.af.hi = 0x00;
    mock_cpu.bc.hi = 0x01; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {
            .zero = false,
            .half_carry = true,
            .carry = true,
        }
    };

    uint8_t opcode = 0x90; // SUB A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x00, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x01, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_sub_a_hl_mem(void) {
    mock_cpu.af.hi = 0x30;
    mock_cpu.hl.reg = 0x20;

    mock_memory[0x20] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x96; // SUB A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
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

        alu_sub8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x1B,
            .status = {
                .zero = false,
                .half_carry = true,
                .carry = false,
            }
        };

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x1B, mock_cpu.af.hi);

        TEST_ASSERT_EQUAL_UINT8(0x20, alu_sub8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x05, alu_sub8_stats.calls[i].value);
        TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[i].carry);
    }
}

void test_op_sub_a_a(void) {
    mock_cpu.af.hi = 0x55;

    uint8_t opcode = 0x97; // SUB A,A

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = false,
            .carry = false,
        }
    };

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x55, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x55, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

// ---- op_sbc_a_r8 ----
void test_op_sbc_a_r8_no_carry(void) {
    mock_cpu.af.hi = 0x30;
    mock_cpu.bc.hi = 0x10; // B
    flag_clear(&mock_cpu, FLAG_C);

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x98; // SBC A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_INT(1, alu_sub8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_sbc_a_r8_with_carry(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x30;
    mock_cpu.bc.hi = 0x10; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x1F,
        .status = {
            .zero = false,
            .half_carry = true,
            .carry = false,
        }
    };

    uint8_t opcode = 0x98; // SBC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x1F, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H)); // 0x0 - 1 → borrow from bit 4
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[0].carry);
}

void test_op_sbc_a_r8_sets_zero_flag(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x42;
    mock_cpu.bc.hi = 0x41; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0x98; // SBC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x42, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x41, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[0].carry);
}

void test_op_sbc_a_r8_half_borrow_edge_case(void) {
    // Classic tricky case: 0x1F - 0x0F - 1
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x1F;
    mock_cpu.bc.hi = 0x0F;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {
            .zero = false,
            .half_carry = true,
            .carry = false,
        }
    };

    uint8_t opcode = 0x98; // SBC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x0F, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H)); // MUST be set
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x1F, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[0].carry);
}

void test_op_sbc_a_r8_sets_carry_and_half_borrow(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x00;
    mock_cpu.bc.hi = 0x00;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {
            .zero = false,
            .half_carry = true,
            .carry = true,
        }
    };

    uint8_t opcode = 0x98; // SBC A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x00, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x00, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[0].carry);
}

void test_op_sbc_a_hl_mem(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x30;
    mock_cpu.hl.reg = 0x20;

    mock_memory[0x20] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x1F,
        .status = {
            .zero = false,
            .half_carry = true,
            .carry = false,
        }
    };

    uint8_t opcode = 0x9E; // SBC A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x1F, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[0].carry);
}

void test_op_sbc_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0x98, &mock_cpu.bc.hi}, // B
        {0x99, &mock_cpu.bc.lo}, // C
        {0x9A, &mock_cpu.de.hi}, // D
        {0x9B, &mock_cpu.de.lo}, // E
        {0x9C, &mock_cpu.hl.hi}, // H
        {0x9D, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        flag_set(&mock_cpu, FLAG_C);
        mock_cpu.af.hi = 0x20;
        *cases[i].reg = 0x05;

        alu_sub8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x1A,
            .status = {
                .zero = false,
                .half_carry = true,
                .carry = false,
            }
        };

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x1A, mock_cpu.af.hi); // 0x20 - 0x05 - 1

        TEST_ASSERT_EQUAL_UINT8(0x20, alu_sub8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x05, alu_sub8_stats.calls[i].value);
        TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[i].carry);
    }
}

void test_op_sbc_a_a(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x55;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {
            .zero = false,
            .half_carry = true,
            .carry = true,
        }
    };

    uint8_t opcode = 0x9F; // SBC A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x55, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x55, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[0].carry);
}

// ---- op_and_a_r8 ----
void test_op_and_a_r8(void) {
    mock_cpu.af.hi = 0xF0;
    mock_cpu.bc.hi = 0x0F; // B

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
        }
    };

    uint8_t opcode = 0xA0; // AND A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_INT(1, alu_and8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_and8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_and8_stats.calls[0].value);
}

void test_op_and_a_r8_non_zero_result(void) {
    mock_cpu.af.hi = 0xF0;
    mock_cpu.bc.hi = 0xF0; // B

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xF0,
        .status = {
            .zero = false,
        }
    };

    uint8_t opcode = 0xA0; // AND A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xF0, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_and8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_and8_stats.calls[0].value);
}

void test_op_and_a_r8_zero_result(void) {
    mock_cpu.af.hi = 0x0F;
    mock_cpu.bc.hi = 0xF0; // B

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
        }
    };

    uint8_t opcode = 0xA0; // AND A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_and8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_and8_stats.calls[0].value);
}

void test_op_and_a_r8_hl_mem(void) {
    mock_cpu.af.hi = 0xF0;
    mock_cpu.hl.reg = 0x1234;
    mock_memory[0x1234] = 0x0F;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x00,
       .status = {
           .zero = true,
       }
    };

    uint8_t opcode = 0xA6; // AND A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_and8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_and8_stats.calls[0].value);
}

void test_op_and_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0xA0, &mock_cpu.bc.hi}, // B
        {0xA1, &mock_cpu.bc.lo}, // C
        {0xA2, &mock_cpu.de.hi}, // D
        {0xA3, &mock_cpu.de.lo}, // E
        {0xA4, &mock_cpu.hl.hi}, // H
        {0xA5, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        mock_cpu.af.hi = 0xAA;
        *cases[i].reg = 0x0F;

        alu_and8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x0A,
            .status = {
                .zero = false,
            }
        };

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x0A, mock_cpu.af.hi);
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
        TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

        TEST_ASSERT_EQUAL_UINT8(0xAA, alu_and8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x0F, alu_and8_stats.calls[i].value);
    }
}

void test_op_and_a_a(void) {
    mock_cpu.af.hi = 0xAA;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0xAA,
       .status = {
           .zero = false,
       }
    };

    uint8_t opcode = 0xA7; // AND A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0xAA, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xAA, alu_and8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0xAA, alu_and8_stats.calls[0].value);
}

// ---- op_xor_a_r8 ----
void test_op_xor_a_r8(void) {
    mock_cpu.af.hi = 0xAA;
    mock_cpu.bc.hi = 0x55; // B

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0xFF,
       .status = {
           .zero = false,
       }
    };

    uint8_t opcode = 0xA8; // XOR A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xAA, alu_xor8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x55, alu_xor8_stats.calls[0].value);
}

void test_op_xor_a_r8_zero_result(void) {
    mock_cpu.af.hi = 0xFF;
    mock_cpu.bc.hi = 0xFF; // B

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x00,
       .status = {
           .zero = true,
       }
    };

    uint8_t opcode = 0xA8; // XOR A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xFF, alu_xor8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0xFF, alu_xor8_stats.calls[0].value);
}

void test_op_xor_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0xA8, &mock_cpu.bc.hi}, // B
        {0xA9, &mock_cpu.bc.lo}, // C
        {0xAA, &mock_cpu.de.hi}, // D
        {0xAB, &mock_cpu.de.lo}, // E
        {0xAC, &mock_cpu.hl.hi}, // H
        {0xAD, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        mock_cpu.af.hi = 0xF0;
        *cases[i].reg = 0x0F;

        alu_xor8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0xFF,
            .status = {
                .zero = false,
            }
        };

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

        TEST_ASSERT_EQUAL_UINT8(0xF0, alu_xor8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x0F, alu_xor8_stats.calls[i].value);
    }
}

void test_op_xor_a_r8_hl_mem(void) {
    mock_cpu.af.hi = 0xF0;
    mock_cpu.hl.reg = 0x2000;
    mock_memory[0x2000] = 0x0F;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {
            .zero = false,
        }
    };

    uint8_t opcode = 0xAE; // XOR A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_xor8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_xor8_stats.calls[0].value);
}

void test_op_xor_a_a(void) {
    mock_cpu.af.hi = 0x5A;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
        }
    };

    uint8_t opcode = 0xAF; // XOR A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x5A, alu_xor8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x5A, alu_xor8_stats.calls[0].value);
}

// ---- op_or_a_r8 ----
void test_op_or_a_r8(void) {
    mock_cpu.af.hi = 0x0F;
    mock_cpu.bc.hi = 0xF0; // B

    alu_or8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0xFF,
       .status = {
           .zero = false,
       }
    };

    uint8_t opcode = 0xB0; // OR A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_INT(1, alu_or8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_or8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_or8_stats.calls[0].value);
}

void test_op_or_a_r8_zero_result(void) {
    mock_cpu.af.hi = 0x00;
    mock_cpu.bc.hi = 0x00; // B

    alu_or8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x00,
       .status = {
           .zero = true,
       }
    };

    uint8_t opcode = 0xB0; // OR A,B

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x00, alu_or8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x00, alu_or8_stats.calls[0].value);
}

void test_op_or_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0xB0, &mock_cpu.bc.hi}, // B
        {0xB1, &mock_cpu.bc.lo}, // C
        {0xB2, &mock_cpu.de.hi}, // D
        {0xB3, &mock_cpu.de.lo}, // E
        {0xB4, &mock_cpu.hl.hi}, // H
        {0xB5, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        mock_cpu.af.hi = 0xAA;
        *cases[i].reg = 0x55;

        alu_or8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0xFF,
            .status = {
                .zero = false,
            }
        };

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

        TEST_ASSERT_EQUAL_UINT8(0xAA, alu_or8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x55, alu_or8_stats.calls[i].value);
    }
}

void test_op_or_a_r8_hl_mem(void) {
    mock_cpu.af.hi = 0x0F;
    mock_cpu.hl.reg = 0x1234;
    mock_memory[0x1234] = 0xF0;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0xFF,
       .status = {
           .zero = false,
       }
    };

    uint8_t opcode = 0xB6; // OR A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_or8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_or8_stats.calls[0].value);
}

void test_op_or_a_a(void) {
    mock_cpu.af.hi = 0x5A;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){
       .value = 0x5A,
       .status = {
           .zero = false,
       }
    };

    uint8_t opcode = 0xB7; // OR A,A

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x5A, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x5A, alu_or8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x5A, alu_or8_stats.calls[0].value);
}

// ---- op_cp_a_r8 ----
void test_op_cp_a_r8(void) {
    mock_cpu.af.hi = 0x30;
    mock_cpu.bc.hi = 0x10; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0xB8; // CP A,B

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(4, cycles);

    // A unchanged
    TEST_ASSERT_EQUAL_UINT8(0x30, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_cp_a_r8_sets_zero(void) {
    mock_cpu.af.hi = 0x42;
    mock_cpu.bc.hi = 0x42; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0xB8;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x42, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x42, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x42, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_cp_a_r8_sets_carry(void) {
    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x20; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xF0,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = true,
        }
    };

    uint8_t opcode = 0xB8;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x10, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x20, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_cp_a_r8_sets_half_borrow(void) {
    mock_cpu.af.hi = 0x10;
    mock_cpu.bc.hi = 0x01; // B

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {
            .zero = false,
            .half_carry = true,
            .carry = false,
        }
    };

    uint8_t opcode = 0xB8;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x10, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H)); // 0x0 - 1 borrow
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x01, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_cp_a_r8_all_registers(void) {
    struct {
        uint8_t opcode;
        uint8_t *reg;
    } cases[] = {
        {0xB8, &mock_cpu.bc.hi}, // B
        {0xB9, &mock_cpu.bc.lo}, // C
        {0xBA, &mock_cpu.de.hi}, // D
        {0xBB, &mock_cpu.de.lo}, // E
        {0xBC, &mock_cpu.hl.hi}, // H
        {0xBD, &mock_cpu.hl.lo}, // L
    };

    for (int i = 0; i < 6; i++) {
        mock_cpu.af.hi = 0x20;
        *cases[i].reg = 0x05;

        alu_sub8_stats.calls[i].return_value = (alu8_result_t){
            .value = 0x1B,
            .status = {
                .zero = false,
                .half_carry = true,
                .carry = false,
            }
        };

        opcode_table[cases[i].opcode](&mock_cpu, &mock_bus, cases[i].opcode);

        TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi); // A unchanged

        TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
        TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));

        TEST_ASSERT_EQUAL_UINT8(0x20, alu_sub8_stats.calls[i].a);
        TEST_ASSERT_EQUAL_UINT8(0x05, alu_sub8_stats.calls[i].value);
        TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[i].carry);
    }
}

void test_op_cp_a_r8_hl_mem(void) {
    mock_cpu.af.hi = 0x30;
    mock_cpu.hl.reg = 0x2000;
    mock_memory[0x2000] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0xBE; // CP A,[HL]

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);

    // A must NOT change
    TEST_ASSERT_EQUAL_UINT8(0x30, mock_cpu.af.hi);

    // 0x30 - 0x10 = 0x20
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));

    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_cp_a_a(void) {
    mock_cpu.af.hi = 0x20;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {
            .zero = true,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0xBF;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi); // A unchanged

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));

    TEST_ASSERT_EQUAL_UINT8(0x20, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x20, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

// ---- op_add_a_imm8 ----

void test_op_add_a_imm8(void) {
    mock_cpu.af.hi = 0x10;
    mock_memory[0] = 0x05;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x15,
        .status = { 0 }
    };

    uint8_t opcode = 0xC6; // ADD A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x15, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_INT(1, alu_add8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x05, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry); // ADD, not ADC

}

void test_op_add_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_memory[0] = 0x00;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xC6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_add_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x01,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xC6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_add_a_imm8_always_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xC6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_add_a_imm8_sets_h_flag_on_half_carry(void) {
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x10,
        .status = {.half_carry = true }
    };

    uint8_t opcode = 0xC6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_add_a_imm8_clears_h_flag_when_no_half_carry(void) {
    flag_set(&mock_cpu, FLAG_H);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x02,
        .status = {.half_carry = false }
    };

    uint8_t opcode = 0xC6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_add_a_imm8_sets_c_flag_on_carry(void) {
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.carry = true }
    };

    uint8_t opcode = 0xC6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_add_a_imm8_clears_c_flag_when_no_carry(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x02,
        .status = {.carry = false }
    };

    uint8_t opcode = 0xC6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_adc_a_imm8 ----

void test_op_adc_a_imm8(void) {
    mock_cpu.af.hi = 0x10;
    mock_memory[0] = 0x05;
    flag_clear(&mock_cpu, FLAG_C);

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x15,
        .status = { 0 }
    };

    uint8_t opcode = 0xCE; // ADC A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x15, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_INT(1, alu_add8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_add8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x05, alu_add8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_add8_stats.calls[0].carry); // no carry flag set
}

void test_op_adc_a_imm8_passes_carry_in(void) {
    mock_cpu.af.hi = 0x10;
    mock_memory[0] = 0x05;
    flag_set(&mock_cpu, FLAG_C);

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x16,
        .status = { 0 }
    };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, alu_add8_stats.calls[0].carry); // carry flag was set
    TEST_ASSERT_EQUAL_UINT8(0x16, mock_cpu.af.hi);
}

void test_op_adc_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_memory[0] = 0x00;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_adc_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x01,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_adc_a_imm8_always_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_adc_a_imm8_sets_h_flag_on_half_carry(void) {
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x10,
        .status = {.half_carry = true }
    };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_adc_a_imm8_clears_h_flag_when_no_half_carry(void) {
    flag_set(&mock_cpu, FLAG_H);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x02,
        .status = {.half_carry = false }
    };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_adc_a_imm8_sets_c_flag_on_carry(void) {
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.carry = true }
    };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_adc_a_imm8_clears_c_flag_when_no_carry(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0] = 0x01;

    alu_add8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x02,
        .status = {.carry = false }
    };

    uint8_t opcode = 0xCE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_sub_a_imm8 ----

void test_op_sub_a_imm8(void) {
    mock_cpu.af.hi = 0x30;
    mock_memory[0] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0xD6; // SUB A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_INT(1, alu_sub8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry); // SUB, not SBC
}

void test_op_sub_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_cpu.af.hi = 0x42;
    mock_memory[0] = 0x42;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xD6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

void test_op_sub_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.af.hi = 0x30;
    mock_memory[0] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xD6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_sub_a_imm8_always_sets_n_flag(void) {
    flag_clear(&mock_cpu, FLAG_N);
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xD6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

void test_op_sub_a_imm8_sets_h_flag_on_half_borrow(void) {
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {.half_carry = true }
    };

    uint8_t opcode = 0xD6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_sub_a_imm8_clears_h_flag_when_no_half_borrow(void) {
    flag_set(&mock_cpu, FLAG_H);
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x01,
        .status = {.half_carry = false }
    };

    uint8_t opcode = 0xD6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_sub_a_imm8_sets_c_flag_on_borrow(void) {
    mock_cpu.af.hi = 0x00;
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {.carry = true }
    };

    uint8_t opcode = 0xD6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sub_a_imm8_clears_c_flag_when_no_borrow(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x10;
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {.carry = false }
    };

    uint8_t opcode = 0xD6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_sbc_a_imm8 ----

void test_op_sbc_a_imm8_no_carry_in(void) {
    mock_cpu.af.hi = 0x30;
    mock_memory[0] = 0x10;
    flag_clear(&mock_cpu, FLAG_C);

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0xDE; // SBC A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x20, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_INT(1, alu_sub8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry); // no carry flag set
}

void test_op_sbc_a_imm8_passes_carry_in(void) {
    mock_cpu.af.hi = 0x30;
    mock_memory[0] = 0x10;
    flag_set(&mock_cpu, FLAG_C);

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x1F,
        .status = { 0 }
    };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, alu_sub8_stats.calls[0].carry); // carry flag was set
    TEST_ASSERT_EQUAL_UINT8(0x1F, mock_cpu.af.hi);
}

void test_op_sbc_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_cpu.af.hi = 0x10;
    mock_memory[0] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

void test_op_sbc_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.af.hi = 0x30;
    mock_memory[0] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_sbc_a_imm8_always_sets_n_flag(void) {
    flag_clear(&mock_cpu, FLAG_N);
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

void test_op_sbc_a_imm8_sets_h_flag_on_half_borrow(void) {
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {.half_carry = true }
    };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_sbc_a_imm8_clears_h_flag_when_no_half_borrow(void) {
    flag_set(&mock_cpu, FLAG_H);
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x01,
        .status = {.half_carry = false }
    };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_sbc_a_imm8_sets_c_flag_on_borrow(void) {
    mock_cpu.af.hi = 0x00;
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {.carry = true }
    };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_sbc_a_imm8_clears_c_flag_when_no_borrow(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x10;
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {.carry = false }
    };

    uint8_t opcode = 0xDE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_xor_a_imm8 ----

void test_op_xor_a_imm8(void) {
    mock_cpu.af.hi = 0xAA;
    mock_memory[0] = 0x55;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xEE; // XOR A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_INT(1, alu_xor8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0xAA, alu_xor8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x55, alu_xor8_stats.calls[0].value);
}

void test_op_xor_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_cpu.af.hi = 0xFF;
    mock_memory[0] = 0xFF;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xEE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_xor_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.af.hi = 0xAA;
    mock_memory[0] = 0x55;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xEE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_xor_a_imm8_always_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N);
    mock_memory[0] = 0x00;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xEE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_xor_a_imm8_always_clears_h_flag(void) {
    flag_set(&mock_cpu, FLAG_H);
    mock_memory[0] = 0x00;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xEE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_xor_a_imm8_always_clears_c_flag(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0] = 0x00;

    alu_xor8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xEE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_and_a_imm8 ----

void test_op_and_a_imm8(void) {
    mock_cpu.af.hi = 0xF0;
    mock_memory[0] = 0x0F;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xE6; // AND A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_INT(1, alu_and8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0xF0, alu_and8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x0F, alu_and8_stats.calls[0].value);
}

void test_op_and_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_cpu.af.hi = 0xF0;
    mock_memory[0] = 0x0F;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xE6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_and_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.af.hi = 0xFF;
    mock_memory[0] = 0x0F;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xE6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_and_a_imm8_always_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N);
    mock_memory[0] = 0xFF;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xE6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_and_a_imm8_always_sets_h_flag(void) {
    flag_clear(&mock_cpu, FLAG_H);
    mock_memory[0] = 0xFF;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xE6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_and_a_imm8_always_clears_c_flag(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0] = 0xFF;

    alu_and8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xE6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_or_a_imm8 ----

void test_op_or_a_imm8(void) {
    mock_cpu.af.hi = 0xA0;
    mock_memory[0] = 0x0B;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xAB,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xF6; // OR A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mock_cpu.af.hi);

    TEST_ASSERT_EQUAL_INT(1, alu_or8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0xA0, alu_or8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x0B, alu_or8_stats.calls[0].value);
}

void test_op_or_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_cpu.af.hi = 0x00;
    mock_memory[0] = 0x00;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xF6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_or_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.af.hi = 0xA0;
    mock_memory[0] = 0x0B;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xAB,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xF6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_or_a_imm8_always_clears_n_flag(void) {
    flag_set(&mock_cpu, FLAG_N);
    mock_memory[0] = 0x00;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xF6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_or_a_imm8_always_clears_h_flag(void) {
    flag_set(&mock_cpu, FLAG_H);
    mock_memory[0] = 0x00;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xF6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_or_a_imm8_always_clears_c_flag(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_memory[0] = 0x00;

    alu_or8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xF6;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_cp_a_imm8 ----

void test_op_cp_a_imm8(void) {
    mock_cpu.af.hi = 0x30;
    mock_memory[0] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {
            .zero = false,
            .half_carry = false,
            .carry = false,
        }
    };

    uint8_t opcode = 0xFE; // CP A, imm8

    int cycles = opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(1, mock_cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x30, mock_cpu.af.hi); // A is NOT modified by CP

    TEST_ASSERT_EQUAL_INT(1, alu_sub8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x30, alu_sub8_stats.calls[0].a);
    TEST_ASSERT_EQUAL_UINT8(0x10, alu_sub8_stats.calls[0].value);
    TEST_ASSERT_EQUAL_UINT8(0, alu_sub8_stats.calls[0].carry);
}

void test_op_cp_a_imm8_sets_z_flag_when_result_is_zero(void) {
    mock_cpu.af.hi = 0x42;
    mock_memory[0] = 0x42;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x00,
        .status = {.zero = true }
    };

    uint8_t opcode = 0xFE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_Z));
    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

void test_op_cp_a_imm8_clears_z_flag_when_result_is_nonzero(void) {
    flag_set(&mock_cpu, FLAG_Z);
    mock_cpu.af.hi = 0x30;
    mock_memory[0] = 0x10;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x20,
        .status = {.zero = false }
    };

    uint8_t opcode = 0xFE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_cp_a_imm8_always_sets_n_flag(void) {
    flag_clear(&mock_cpu, FLAG_N);
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){ 0 };

    uint8_t opcode = 0xFE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_N));
}

void test_op_cp_a_imm8_sets_h_flag_on_half_borrow(void) {
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {.half_carry = true }
    };

    uint8_t opcode = 0xFE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
}

void test_op_cp_a_imm8_clears_h_flag_when_no_half_borrow(void) {
    flag_set(&mock_cpu, FLAG_H);
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x01,
        .status = {.half_carry = false }
    };

    uint8_t opcode = 0xFE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_cp_a_imm8_sets_c_flag_on_borrow(void) {
    mock_cpu.af.hi = 0x00;
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0xFF,
        .status = {.carry = true }
    };

    uint8_t opcode = 0xFE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_cp_a_imm8_clears_c_flag_when_no_borrow(void) {
    flag_set(&mock_cpu, FLAG_C);
    mock_cpu.af.hi = 0x10;
    mock_memory[0] = 0x01;

    alu_sub8_stats.calls[0].return_value = (alu8_result_t){
        .value = 0x0F,
        .status = {.carry = false }
    };

    uint8_t opcode = 0xFE;

    opcode_table[opcode](&mock_cpu, &mock_bus, opcode);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_add_sp_imm8 ----
void test_op_add_sp_imm8_positive_offset(void) {
    mock_cpu.sp = 0x0100;
    mock_memory[0] = 0x05; // offset +5

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0105, .status = {.half_carry = false, .carry = false }
    };

    int cycles = opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0105, mock_cpu.sp);
    TEST_ASSERT_EQUAL_INT(1, alu_add16_s8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT16(0x0100, alu_add16_s8_stats.calls[0].base);
    TEST_ASSERT_EQUAL_INT8(5, alu_add16_s8_stats.calls[0].offset);
}

void test_op_add_sp_imm8_negative_offset(void) {
    mock_cpu.sp = 0x0100;
    mock_memory[0] = 0xFF; // offset -1

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x00FF, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT16(0x00FF, mock_cpu.sp);
    TEST_ASSERT_EQUAL_INT8(-1, alu_add16_s8_stats.calls[0].offset);
}

void test_op_add_sp_imm8_negative_offset_wraps_sp(void) {
    // SP=0x0000, offset=-1 → SP wraps to 0xFFFF
    mock_cpu.sp = 0x0000;
    mock_memory[0] = 0xFF; // offset -1

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0xFFFF, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT16(0xFFFF, mock_cpu.sp);
}

void test_op_add_sp_imm8_always_clears_z_flag(void) {
    // Even when the result is 0, Z must be cleared
    mock_cpu.sp = 0x0001;
    mock_memory[0] = 0xFF; // offset -1 → result 0x0000
    flag_set(&mock_cpu, FLAG_Z);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0000, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_add_sp_imm8_always_clears_n_flag(void) {
    mock_cpu.sp = 0x0100;
    mock_memory[0] = 0x01;
    flag_set(&mock_cpu, FLAG_N);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0101, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_add_sp_imm8_sets_h_flag_on_lower_nibble_carry(void) {
    // SP=0x000F, offset=+1: (0xF & 0xF) + (0x1 & 0xF) = 0x10 > 0xF → H=1
    mock_cpu.sp = 0x000F;
    mock_memory[0] = 0x01;

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0010, .status = {.half_carry = true, .carry = false }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_add_sp_imm8_clears_h_flag_when_no_lower_nibble_carry(void) {
    mock_cpu.sp = 0x0010;
    mock_memory[0] = 0x01;
    flag_set(&mock_cpu, FLAG_H);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0011, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_add_sp_imm8_sets_c_flag_on_lower_byte_carry(void) {
    // SP=0x00FF, offset=+1: 0xFF + 0x01 = 0x100 > 0xFF → C=1
    mock_cpu.sp = 0x00FF;
    mock_memory[0] = 0x01;

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0100, .status = {.half_carry = true, .carry = true }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_add_sp_imm8_clears_c_flag_when_no_lower_byte_carry(void) {
    mock_cpu.sp = 0x0080;
    mock_memory[0] = 0x01;
    flag_set(&mock_cpu, FLAG_C);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0081, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xE8](&mock_cpu, &mock_bus, 0xE8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_ld_hl_sp_plus_imm8 ----
void test_op_ld_hl_sp_plus_imm8_positive_offset(void) {
    mock_cpu.sp = 0x0100;
    mock_memory[0] = 0x05; // offset +5

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0105, .status = {.half_carry = false, .carry = false }
    };

    int cycles = opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0105, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT16(0x0100, mock_cpu.sp); // SP unchanged
    TEST_ASSERT_EQUAL_INT(1, alu_add16_s8_stats.call_count);
    TEST_ASSERT_EQUAL_UINT16(0x0100, alu_add16_s8_stats.calls[0].base);
    TEST_ASSERT_EQUAL_INT8(5, alu_add16_s8_stats.calls[0].offset);
}

void test_op_ld_hl_sp_plus_imm8_negative_offset(void) {
    mock_cpu.sp = 0x0100;
    mock_memory[0] = 0xFF; // offset -1

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x00FF, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT16(0x00FF, mock_cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT16(0x0100, mock_cpu.sp); // SP unchanged
    TEST_ASSERT_EQUAL_INT8(-1, alu_add16_s8_stats.calls[0].offset);
}

void test_op_ld_hl_sp_plus_imm8_does_not_modify_sp(void) {
    mock_cpu.sp = 0xFFFE;
    mock_memory[0] = 0x10; // offset +16

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x000E, .status = {.half_carry = false, .carry = true }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT16(0xFFFE, mock_cpu.sp);
}

void test_op_ld_hl_sp_plus_imm8_always_clears_z_flag(void) {
    mock_cpu.sp = 0x0001;
    mock_memory[0] = 0xFF; // offset -1 → result 0x0000
    flag_set(&mock_cpu, FLAG_Z);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0000, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_Z));
}

void test_op_ld_hl_sp_plus_imm8_always_clears_n_flag(void) {
    mock_cpu.sp = 0x0100;
    mock_memory[0] = 0x01;
    flag_set(&mock_cpu, FLAG_N);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0101, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_N));
}

void test_op_ld_hl_sp_plus_imm8_sets_h_flag_on_lower_nibble_carry(void) {
    mock_cpu.sp = 0x000F;
    mock_memory[0] = 0x01;

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0010, .status = {.half_carry = true, .carry = false }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_H));
    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

void test_op_ld_hl_sp_plus_imm8_clears_h_flag_when_no_lower_nibble_carry(void) {
    mock_cpu.sp = 0x0010;
    mock_memory[0] = 0x01;
    flag_set(&mock_cpu, FLAG_H);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0011, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_H));
}

void test_op_ld_hl_sp_plus_imm8_sets_c_flag_on_lower_byte_carry(void) {
    mock_cpu.sp = 0x00FF;
    mock_memory[0] = 0x01;

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0100, .status = {.half_carry = true, .carry = true }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT8(1, flag_get(&mock_cpu, FLAG_C));
}

void test_op_ld_hl_sp_plus_imm8_clears_c_flag_when_no_lower_byte_carry(void) {
    mock_cpu.sp = 0x0080;
    mock_memory[0] = 0x01;
    flag_set(&mock_cpu, FLAG_C);

    alu_add16_s8_stats.calls[0].return_value = (alu16_result_t){
        .value = 0x0081, .status = {.half_carry = false, .carry = false }
    };

    opcode_table[0xF8](&mock_cpu, &mock_bus, 0xF8);

    TEST_ASSERT_EQUAL_UINT8(0, flag_get(&mock_cpu, FLAG_C));
}

// ---- op_ld_sp_hl ----
void test_op_ld_sp_hl_loads_hl_into_sp(void) {
    mock_cpu.hl.reg = 0x1234;
    mock_cpu.sp = 0x0000;

    int cycles = opcode_table[0xF9](&mock_cpu, &mock_bus, 0xF9);

    TEST_ASSERT_EQUAL(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x1234, mock_cpu.sp);
}

void test_op_ld_sp_hl_does_not_modify_hl(void) {
    mock_cpu.hl.reg = 0xABCD;
    mock_cpu.sp = 0x0000;

    opcode_table[0xF9](&mock_cpu, &mock_bus, 0xF9);

    TEST_ASSERT_EQUAL_UINT16(0xABCD, mock_cpu.hl.reg);
}

// ---- op_di ----
void test_op_di_clears_ime(void) {
    mock_cpu.ime.enabled = true;

    int cycles = opcode_table[0xF3](&mock_cpu, &mock_bus, 0xF3);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_FALSE(mock_cpu.ime.enabled);
}

void test_op_di_clears_ime_when_already_false(void) {
    mock_cpu.ime.enabled = false;

    opcode_table[0xF3](&mock_cpu, &mock_bus, 0xF3);

    TEST_ASSERT_FALSE(mock_cpu.ime.enabled);
}

// ---- op_ei ----
void test_op_ei_schedules_ime(void) {
    mock_cpu.ime.scheduled = false;

    int cycles = opcode_table[0xFB](&mock_cpu, &mock_bus, 0xFB);

    TEST_ASSERT_EQUAL(4, cycles);
    TEST_ASSERT_TRUE(mock_cpu.ime.scheduled);
}

void test_op_ei_does_not_set_ime_immediately(void) {
    // EI delays by one instruction — IME must NOT be set right away
    mock_cpu.ime.enabled = false;
    mock_cpu.ime.scheduled = false;

    opcode_table[0xFB](&mock_cpu, &mock_bus, 0xFB);

    TEST_ASSERT_FALSE(mock_cpu.ime.enabled);
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
    RUN_TEST(test_op_add_hl_r16_all_registers);
    RUN_TEST(test_op_add_hl_hl);
    RUN_TEST(test_op_add_hl_r16_reset_nhc_flags_if_no_overflow);
    RUN_TEST(test_op_add_hl_r16_set_half_carry_if_overflow_from_bit_11);
    RUN_TEST(test_op_add_hl_r16_set_carry_if_overflow_from_bit_15);
    RUN_TEST(test_op_inc_hl_mem);
    RUN_TEST(test_op_inc_r8_sets_z_flag_on_overflow);
    RUN_TEST(test_op_inc_r8_sets_h_flag_on_nibble_overflow);
    RUN_TEST(test_op_inc_r8_does_not_set_h_flag_when_no_nibble_overflow);
    RUN_TEST(test_op_inc_r8_clears_n_flag);
    RUN_TEST(test_op_dec_r8_all_registers);
    RUN_TEST(test_op_dec_hl_mem);
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
    RUN_TEST(test_op_rlc_r8_matrix);
    RUN_TEST(test_op_rlc_r8_hl_mem);
    RUN_TEST(test_op_rlc_r8_sets_carry_when_msb_is_1);
    RUN_TEST(test_op_rlc_r8_clears_carry_when_msb_is_0);
    RUN_TEST(test_op_rlc_r8_sets_zero_flag_when_result_is_zero);
    RUN_TEST(test_op_rlc_r8_clears_zero_flag_when_result_is_nonzero);
    RUN_TEST(test_op_rlc_r8_clears_n_and_h_flags);
    RUN_TEST(test_op_rrc_r8_matrix);
    RUN_TEST(test_op_rrc_r8_hl_mem);
    RUN_TEST(test_op_rrc_r8_sets_carry_when_lsb_is_1);
    RUN_TEST(test_op_rrc_r8_clears_carry_when_lsb_is_0);
    RUN_TEST(test_op_rrc_r8_sets_zero_flag_when_result_is_zero);
    RUN_TEST(test_op_rrc_r8_clears_zero_flag_when_result_is_nonzero);
    RUN_TEST(test_op_rrc_r8_clears_n_and_h_flags);
    RUN_TEST(test_op_rl_r8_matrix);
    RUN_TEST(test_op_rl_r8_hl_mem);
    RUN_TEST(test_op_rl_r8_inserts_carry_into_bit0);
    RUN_TEST(test_op_rl_r8_sets_carry_when_msb_is_1);
    RUN_TEST(test_op_rl_r8_clears_carry_when_msb_is_0);
    RUN_TEST(test_op_rl_r8_sets_zero_flag_when_result_is_zero);
    RUN_TEST(test_op_rl_r8_clears_zero_flag_when_result_is_nonzero);
    RUN_TEST(test_op_rl_r8_clears_n_and_h_flags);
    RUN_TEST(test_op_rr_r8_matrix);
    RUN_TEST(test_op_rr_r8_hl_mem);
    RUN_TEST(test_op_rr_r8_inserts_carry_into_bit7);
    RUN_TEST(test_op_rr_r8_sets_carry_when_lsb_is_1);
    RUN_TEST(test_op_rr_r8_clears_carry_when_lsb_is_0);
    RUN_TEST(test_op_rr_r8_sets_zero_flag_when_result_is_zero);
    RUN_TEST(test_op_rr_r8_clears_zero_flag_when_result_is_nonzero);
    RUN_TEST(test_op_rr_r8_clears_n_and_h_flags);
    RUN_TEST(test_op_sla_r8_matrix);
    RUN_TEST(test_op_sla_r8_hl_mem);
    RUN_TEST(test_op_sla_r8_bit0_is_always_zero);
    RUN_TEST(test_op_sla_r8_sets_carry_when_msb_is_1);
    RUN_TEST(test_op_sla_r8_clears_carry_when_msb_is_0);
    RUN_TEST(test_op_sla_r8_sets_zero_flag_when_result_is_zero);
    RUN_TEST(test_op_sla_r8_clears_zero_flag_when_result_is_nonzero);
    RUN_TEST(test_op_sla_r8_clears_n_and_h_flags);
    RUN_TEST(test_op_sra_r8_matrix);
    RUN_TEST(test_op_sra_r8_hl_mem);
    RUN_TEST(test_op_sra_r8_preserves_msb);
    RUN_TEST(test_op_sra_r8_sets_carry_when_lsb_is_1);
    RUN_TEST(test_op_sra_r8_clears_carry_when_lsb_is_0);
    RUN_TEST(test_op_sra_r8_sets_zero_flag_when_result_is_zero);
    RUN_TEST(test_op_sra_r8_clears_zero_flag_when_result_is_nonzero);
    RUN_TEST(test_op_sra_r8_clears_n_and_h_flags);
    RUN_TEST(test_op_swap_r8_matrix);
    RUN_TEST(test_op_swap_r8_hl_mem);
    RUN_TEST(test_op_swap_r8_sets_zero_flag_when_result_is_zero);
    RUN_TEST(test_op_swap_r8_clears_zero_flag_when_result_is_nonzero);
    RUN_TEST(test_op_swap_r8_always_clears_n_h_c_flags);
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
    RUN_TEST(test_op_ret_nz_condition_true);
    RUN_TEST(test_op_ret_nz_condition_false);
    RUN_TEST(test_op_ret_z_condition_true);
    RUN_TEST(test_op_ret_z_condition_false);
    RUN_TEST(test_op_ret_nc_condition_true);
    RUN_TEST(test_op_ret_c_condition_true);
    RUN_TEST(test_op_ret_loads_pc_from_stack);
    RUN_TEST(test_op_ret_cycles_independent_of_flags);
    RUN_TEST(test_op_reti_loads_pc_from_stack);
    RUN_TEST(test_op_reti_sets_ime_immediately);
    RUN_TEST(test_op_reti_sets_ime_even_when_not_scheduled);
    RUN_TEST(test_op_jp_imm16_jumps_to_address);
    RUN_TEST(test_op_jp_imm16_high_address);
    RUN_TEST(test_op_jp_nz_condition_true);
    RUN_TEST(test_op_jp_nz_condition_false);
    RUN_TEST(test_op_jp_z_condition_true);
    RUN_TEST(test_op_jp_nc_condition_true);
    RUN_TEST(test_op_jp_c_condition_true);
    RUN_TEST(test_op_jp_cond_high_address);
    RUN_TEST(test_op_jp_hl_jumps_to_hl);
    RUN_TEST(test_op_jp_hl_does_not_modify_hl);
    RUN_TEST(test_op_jp_hl_high_address);
    RUN_TEST(test_op_call_imm16_jumps_to_address);
    RUN_TEST(test_op_call_imm16_pushes_return_address);
    RUN_TEST(test_op_call_imm16_return_address_roundtrip);
    RUN_TEST(test_op_call_nz_condition_true);
    RUN_TEST(test_op_call_nz_condition_false);
    RUN_TEST(test_op_call_z_condition_true);
    RUN_TEST(test_op_call_nc_condition_true);
    RUN_TEST(test_op_call_c_condition_true);
    RUN_TEST(test_op_rst_tgt3_matrix);
    RUN_TEST(test_op_rst_return_address_roundtrip);
    RUN_TEST(test_op_pop_r16stk_matrix);
    RUN_TEST(test_op_pop_af_loads_flags);
    RUN_TEST(test_op_pop_af_masks_lower_nibble_of_f);
    RUN_TEST(test_op_push_r16stk_matrix);
    RUN_TEST(test_op_push_af_stores_flags);
    RUN_TEST(test_op_push_af_masks_lower_nibble_of_f);
    RUN_TEST(test_op_ldh_imm8mem_a_writes_a_to_hram);
    RUN_TEST(test_op_ldh_imm8mem_a_uses_ff00_base);
    RUN_TEST(test_op_ldh_c_mem_a_writes_a_to_hram);
    RUN_TEST(test_op_ldh_c_mem_a_uses_ff00_base);
    RUN_TEST(test_op_ld_imm16mem_a_writes_a_to_address);
    RUN_TEST(test_op_ldh_a_imm8mem_reads_from_hram);
    RUN_TEST(test_op_ldh_a_imm8mem_uses_ff00_base);
    RUN_TEST(test_op_ldh_a_c_mem_reads_from_hram);
    RUN_TEST(test_op_ldh_a_c_mem_uses_ff00_base);
    RUN_TEST(test_op_ld_a_imm16mem_reads_from_address);
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
    RUN_TEST(test_op_adc_a_r8_sets_half_carry);
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
    RUN_TEST(test_op_sbc_a_r8_no_carry);
    RUN_TEST(test_op_sbc_a_r8_with_carry);
    RUN_TEST(test_op_sbc_a_r8_sets_zero_flag);
    RUN_TEST(test_op_sbc_a_r8_half_borrow_edge_case);
    RUN_TEST(test_op_sbc_a_r8_sets_carry_and_half_borrow);
    RUN_TEST(test_op_sbc_a_hl_mem);
    RUN_TEST(test_op_sbc_a_r8_all_registers);
    RUN_TEST(test_op_sbc_a_a);
    RUN_TEST(test_op_and_a_r8);
    RUN_TEST(test_op_and_a_r8_non_zero_result);
    RUN_TEST(test_op_and_a_r8_zero_result);
    RUN_TEST(test_op_and_a_r8_hl_mem);
    RUN_TEST(test_op_and_a_r8_all_registers);
    RUN_TEST(test_op_and_a_a);
    RUN_TEST(test_op_xor_a_r8);
    RUN_TEST(test_op_xor_a_r8_zero_result);
    RUN_TEST(test_op_xor_a_r8_all_registers);
    RUN_TEST(test_op_xor_a_r8_hl_mem);
    RUN_TEST(test_op_xor_a_a);
    RUN_TEST(test_op_or_a_r8);
    RUN_TEST(test_op_or_a_r8_zero_result);
    RUN_TEST(test_op_or_a_r8_all_registers);
    RUN_TEST(test_op_or_a_r8_hl_mem);
    RUN_TEST(test_op_or_a_a);
    RUN_TEST(test_op_cp_a_r8);
    RUN_TEST(test_op_cp_a_r8_sets_zero);
    RUN_TEST(test_op_cp_a_r8_sets_carry);
    RUN_TEST(test_op_cp_a_r8_sets_half_borrow);
    RUN_TEST(test_op_cp_a_r8_all_registers);
    RUN_TEST(test_op_cp_a_r8_hl_mem);
    RUN_TEST(test_op_cp_a_a);
    RUN_TEST(test_op_add_a_imm8);
    RUN_TEST(test_op_add_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_add_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_add_a_imm8_always_clears_n_flag);
    RUN_TEST(test_op_add_a_imm8_sets_h_flag_on_half_carry);
    RUN_TEST(test_op_add_a_imm8_clears_h_flag_when_no_half_carry);
    RUN_TEST(test_op_add_a_imm8_sets_c_flag_on_carry);
    RUN_TEST(test_op_add_a_imm8_clears_c_flag_when_no_carry);
    RUN_TEST(test_op_adc_a_imm8);
    RUN_TEST(test_op_adc_a_imm8_passes_carry_in);
    RUN_TEST(test_op_adc_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_adc_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_adc_a_imm8_always_clears_n_flag);
    RUN_TEST(test_op_adc_a_imm8_sets_h_flag_on_half_carry);
    RUN_TEST(test_op_adc_a_imm8_clears_h_flag_when_no_half_carry);
    RUN_TEST(test_op_adc_a_imm8_sets_c_flag_on_carry);
    RUN_TEST(test_op_adc_a_imm8_clears_c_flag_when_no_carry);
    RUN_TEST(test_op_sub_a_imm8);
    RUN_TEST(test_op_sub_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_sub_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_sub_a_imm8_always_sets_n_flag);
    RUN_TEST(test_op_sub_a_imm8_sets_h_flag_on_half_borrow);
    RUN_TEST(test_op_sub_a_imm8_clears_h_flag_when_no_half_borrow);
    RUN_TEST(test_op_sub_a_imm8_sets_c_flag_on_borrow);
    RUN_TEST(test_op_sub_a_imm8_clears_c_flag_when_no_borrow);
    RUN_TEST(test_op_sbc_a_imm8_no_carry_in);
    RUN_TEST(test_op_sbc_a_imm8_passes_carry_in);
    RUN_TEST(test_op_sbc_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_sbc_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_sbc_a_imm8_always_sets_n_flag);
    RUN_TEST(test_op_sbc_a_imm8_sets_h_flag_on_half_borrow);
    RUN_TEST(test_op_sbc_a_imm8_clears_h_flag_when_no_half_borrow);
    RUN_TEST(test_op_sbc_a_imm8_sets_c_flag_on_borrow);
    RUN_TEST(test_op_sbc_a_imm8_clears_c_flag_when_no_borrow);
    RUN_TEST(test_op_and_a_imm8);
    RUN_TEST(test_op_and_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_and_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_and_a_imm8_always_clears_n_flag);
    RUN_TEST(test_op_and_a_imm8_always_sets_h_flag);
    RUN_TEST(test_op_and_a_imm8_always_clears_c_flag);
    RUN_TEST(test_op_xor_a_imm8);
    RUN_TEST(test_op_xor_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_xor_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_xor_a_imm8_always_clears_n_flag);
    RUN_TEST(test_op_xor_a_imm8_always_clears_h_flag);
    RUN_TEST(test_op_xor_a_imm8_always_clears_c_flag);
    RUN_TEST(test_op_or_a_imm8);
    RUN_TEST(test_op_or_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_or_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_or_a_imm8_always_clears_n_flag);
    RUN_TEST(test_op_or_a_imm8_always_clears_h_flag);
    RUN_TEST(test_op_or_a_imm8_always_clears_c_flag);
    RUN_TEST(test_op_cp_a_imm8);
    RUN_TEST(test_op_cp_a_imm8_sets_z_flag_when_result_is_zero);
    RUN_TEST(test_op_cp_a_imm8_clears_z_flag_when_result_is_nonzero);
    RUN_TEST(test_op_cp_a_imm8_always_sets_n_flag);
    RUN_TEST(test_op_cp_a_imm8_sets_h_flag_on_half_borrow);
    RUN_TEST(test_op_cp_a_imm8_clears_h_flag_when_no_half_borrow);
    RUN_TEST(test_op_cp_a_imm8_sets_c_flag_on_borrow);
    RUN_TEST(test_op_cp_a_imm8_clears_c_flag_when_no_borrow);
    RUN_TEST(test_op_add_sp_imm8_positive_offset);
    RUN_TEST(test_op_add_sp_imm8_negative_offset);
    RUN_TEST(test_op_add_sp_imm8_negative_offset_wraps_sp);
    RUN_TEST(test_op_add_sp_imm8_always_clears_z_flag);
    RUN_TEST(test_op_add_sp_imm8_always_clears_n_flag);
    RUN_TEST(test_op_add_sp_imm8_sets_h_flag_on_lower_nibble_carry);
    RUN_TEST(test_op_add_sp_imm8_clears_h_flag_when_no_lower_nibble_carry);
    RUN_TEST(test_op_add_sp_imm8_sets_c_flag_on_lower_byte_carry);
    RUN_TEST(test_op_add_sp_imm8_clears_c_flag_when_no_lower_byte_carry);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_positive_offset);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_negative_offset);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_does_not_modify_sp);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_always_clears_z_flag);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_always_clears_n_flag);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_sets_h_flag_on_lower_nibble_carry);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_clears_h_flag_when_no_lower_nibble_carry);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_sets_c_flag_on_lower_byte_carry);
    RUN_TEST(test_op_ld_hl_sp_plus_imm8_clears_c_flag_when_no_lower_byte_carry);
    RUN_TEST(test_op_ld_sp_hl_loads_hl_into_sp);
    RUN_TEST(test_op_ld_sp_hl_does_not_modify_hl);
    RUN_TEST(test_op_di_clears_ime);
    RUN_TEST(test_op_di_clears_ime_when_already_false);
    RUN_TEST(test_op_ei_schedules_ime);
    RUN_TEST(test_op_ei_does_not_set_ime_immediately);

    return UNITY_END();
}
