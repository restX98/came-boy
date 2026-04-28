#include "cpu.h"

#include "logger.h"
#include "opcodes.h"

void cpu_init(cpu_t *cpu) {
    LOG_INFO("Initializing CPU");

    cpu->af.hi = 0x01;
    cpu->af.lo = 0xB0; // Z=1, N=0, H=1, C=1
    cpu->bc.reg = 0x0013;
    cpu->de.reg = 0x00D8;
    cpu->hl.reg = 0x014D;
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0100;

    LOG_DEBUG(
        "CPU initialized:\n"
        "  AF=0x%04X\n"
        "  BC=0x%04X\n"
        "  DE=0x%04X\n"
        "  HL=0x%04X\n"
        "  SP=0x%04X\n"
        "  PC=0x%04X",
        cpu->af.reg, cpu->bc.reg, cpu->de.reg,
        cpu->hl.reg, cpu->sp, cpu->pc
    );
}

int cpu_step(cpu_t *cpu, bus_t *bus) {
    uint8_t instruction = bus_read(bus, cpu->pc);
    LOG_DEBUG("PC=0x%04X opcode=0x%02X", cpu->pc, instruction);

    opcode_fn fn = opcode_table[instruction];
    if (fn) {
        cpu->pc++;
        int cycles = fn(cpu, bus, instruction);
        LOG_DEBUG("Executed opcode 0x%02X in %d cycles", instruction, cycles);
        return cycles;
    }

    LOG_ERROR("Unknown opcode 0x%02X at PC=0x%04X - halting", instruction, cpu->pc);
    return -1;
}
