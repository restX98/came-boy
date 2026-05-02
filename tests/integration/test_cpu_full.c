#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "bus.h"
#include "cpu.h"
#include "mem.h"
#include "opcodes.h"

static cpu_t   cpu;
static bus_t   bus;
static cartridge_t cartridge;
static uint8_t fake_rom[0x8000];

void setUp(void) {
    suppress_logs();

    memset(fake_rom, 0, sizeof(fake_rom));
    cartridge = (cartridge_t){ .rom = fake_rom, .size = sizeof(fake_rom), .bank = 1 };
    bus_init(&bus, &cartridge);
    cpu_init(&cpu);
}

void tearDown(void) {
    bus_free(&bus);
    restore_logs();
}

// ---- helpers ----

static void load_rom(uint16_t addr, uint8_t *bytes, size_t len) {
    memcpy(&fake_rom[addr], bytes, len);
}

// ---- NOP (0x00) ----

void test_nop_returns_4_cycles(void) {
    uint8_t program[] = { 0x00 };
    load_rom(cpu.pc, program, sizeof(program));

    int cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(4, cycles);
}

void test_nop_advances_pc_by_1(void) {
    uint8_t program[] = { 0x00 };
    load_rom(cpu.pc, program, sizeof(program));

    uint16_t pc_before = cpu.pc;
    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(pc_before + 1, cpu.pc);
}

void test_nop_does_not_modify_registers(void) {
    uint8_t program[] = { 0x00 };
    load_rom(cpu.pc, program, sizeof(program));

    uint16_t af = cpu.af.reg;
    uint16_t bc = cpu.bc.reg;
    uint16_t de = cpu.de.reg;
    uint16_t hl = cpu.hl.reg;
    uint16_t sp = cpu.sp;

    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(af, cpu.af.reg);
    TEST_ASSERT_EQUAL_UINT16(bc, cpu.bc.reg);
    TEST_ASSERT_EQUAL_UINT16(de, cpu.de.reg);
    TEST_ASSERT_EQUAL_UINT16(hl, cpu.hl.reg);
    TEST_ASSERT_EQUAL_UINT16(sp, cpu.sp);
}

// ---- LD r16, d16 ----

void test_ld_bc_d16_loads_immediate_value(void) {
    uint8_t program[] = { 0x01, 0x34, 0x12 }; // LD BC, 0x1234
    load_rom(cpu.pc, program, sizeof(program));

    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.bc.reg);
}

void test_ld_sp_d16_loads_immediate_value(void) {
    uint8_t program[] = { 0x31, 0xFE, 0xFF }; // LD SP, 0xFFFE
    load_rom(cpu.pc, program, sizeof(program));

    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(0xFFFE, cpu.sp);
}

void test_ld_r16_d16_returns_12_cycles(void) {
    uint8_t program[] = { 0x01, 0x00, 0x00 }; // LD BC, 0x0000
    load_rom(cpu.pc, program, sizeof(program));

    int cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(12, cycles);
}

void test_ld_r16_d16_advances_pc_by_3(void) {
    uint8_t program[] = { 0x01, 0x34, 0x12 }; // LD BC, 0x1234
    load_rom(cpu.pc, program, sizeof(program));

    uint16_t pc_before = cpu.pc;
    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(pc_before + 3, cpu.pc);
}

// ---- unknown opcode ----

void test_unknown_opcode_returns_minus1(void) {
    uint8_t program[] = { 0xD3 }; // undefined opcode
    load_rom(cpu.pc, program, sizeof(program));

    int result = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_unknown_opcode_does_not_advance_pc(void) {
    uint8_t program[] = { 0xD3 };
    load_rom(cpu.pc, program, sizeof(program));

    uint16_t pc_before = cpu.pc;
    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(pc_before, cpu.pc);
}

// ---- sequential execution ----

void test_two_nops_advance_pc_by_2(void) {
    uint8_t program[] = { 0x00, 0x00 }; // NOP, NOP
    load_rom(cpu.pc, program, sizeof(program));

    uint16_t pc_before = cpu.pc;
    cpu_step(&cpu, &bus);
    cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(pc_before + 2, cpu.pc);
}

void test_ld_followed_by_nop(void) {
    uint8_t program[] = { 0x01, 0x34, 0x12, 0x00 }; // LD BC,0x1234 then NOP
    load_rom(cpu.pc, program, sizeof(program));

    uint16_t pc_before = cpu.pc;
    cpu_step(&cpu, &bus); // LD BC, 0x1234
    cpu_step(&cpu, &bus); // NOP

    TEST_ASSERT_EQUAL_UINT16(0x1234, cpu.bc.reg);
    TEST_ASSERT_EQUAL_UINT16(pc_before + 4, cpu.pc);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_nop_returns_4_cycles);
    RUN_TEST(test_nop_advances_pc_by_1);
    RUN_TEST(test_nop_does_not_modify_registers);
    RUN_TEST(test_ld_bc_d16_loads_immediate_value);
    RUN_TEST(test_ld_sp_d16_loads_immediate_value);
    RUN_TEST(test_ld_r16_d16_returns_12_cycles);
    RUN_TEST(test_ld_r16_d16_advances_pc_by_3);
    RUN_TEST(test_unknown_opcode_returns_minus1);
    RUN_TEST(test_unknown_opcode_does_not_advance_pc);
    RUN_TEST(test_two_nops_advance_pc_by_2);
    RUN_TEST(test_ld_followed_by_nop);

    return UNITY_END();
}
