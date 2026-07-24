#include "cpu.h"

#include "logger.h"
#include "isa/opcodes.h"

static int cpu_dispatch(cpu_t *cpu, bus_t *bus);
static int interrupt_service_routine(cpu_t *cpu, bus_t *bus, int pending);

void cpu_init(cpu_t *cpu, interrupt_regs_t *interrupts, gb_clock_t *clock) {
    LOG_INFO("Initializing CPU");

    cpu->interrupts = interrupts;
    cpu->clock = clock;

    cpu->af.hi = 0x01;
    cpu->af.lo = 0xB0; // Z=1, N=0, H=1, C=1
    cpu->bc.reg = 0x0013;
    cpu->de.reg = 0x00D8;
    cpu->hl.reg = 0x014D;
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0100;

    cpu->ime.enabled = false;
    cpu->ime.scheduled = false;

    cpu->halted = false;
    cpu->halt_bug = false;

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
    cpu->cycles_ticked = 0;

    int cycles = cpu_dispatch(cpu, bus);

    if (cycles > 0) {
        int remaining = cycles - cpu->cycles_ticked;
        if (remaining > 0) {
            clock_tick(cpu->clock, remaining);
        }
    }

    return cycles;
}

static int cpu_dispatch(cpu_t *cpu, bus_t *bus) {
    int pending = interrupts_pending(cpu->interrupts);
    if (pending >= 0 && cpu->ime.enabled) {
        return interrupt_service_routine(cpu, bus, pending);
    }

    if (cpu->halted) {
        // A halted CPU still runs one M-cycle per step, advancing the machine
        // and possibly raising an interrupt. Tick FIRST, then re-check IF & IE,
        // so the wake lands on the same M-cycle the flag is raised.
        cpu_tick(cpu);

        if (interrupts_pending(cpu->interrupts) >= 0) {
            // Pending IRQs wake the CPU from HALT even when IME=0.
            LOG_DEBUG("Waking from HALT: re-checked IF & IE after tick");
            cpu->halted = false;
        }
        return 4; // The M-cycle spent (still) halted
    }

    // EI delays enabling IME by one instruction. It sets ime_scheduled, and we
    // promote it to ime here, at the START of the following step, before executing
    // the next instruction. This guarantees that instruction runs uninterrupted,
    // and interrupts can only fire from the step after that.
    if (cpu->ime.scheduled) {
        cpu->ime.enabled = true;
        cpu->ime.scheduled = false;
    }

    uint8_t instruction = cpu_read(cpu, bus, cpu->pc);
    LOG_DEBUG("PC=0x%04X opcode=0x%02X", cpu->pc, instruction);

    opcode_fn fn = opcode_table[instruction];
    if (fn) {
        if (cpu->halt_bug) {
            cpu->halt_bug = false;
        } else {
            cpu->pc++;
        }
        int cycles = fn(cpu, bus, instruction);
        LOG_DEBUG("Executed opcode 0x%02X in %d cycles", instruction, cycles);
        return cycles;
    }

    LOG_ERROR("Unknown opcode 0x%02X at PC=0x%04X - halting", instruction, cpu->pc);
    return -1;
}

static int interrupt_service_routine(cpu_t *cpu, bus_t *bus, int pending) {
    static const uint16_t handlers[] = { 0x40, 0x48, 0x50, 0x58, 0x60 };

    // Dispatching an interrupt always wakes the CPU from HALT.
    cpu->halted = false;
    cpu->ime.enabled = false;

    cpu_write(cpu, bus, cpu->sp - 1, cpu->pc >> 8);

    // Re-check pending AFTER high byte push, since IE may have changed
    pending = interrupts_pending(cpu->interrupts);
    if (pending < 0) {
        // IE was cleared by the push — corrupted dispatch
        cpu_write(cpu, bus, cpu->sp - 2, 0x00);
        cpu->sp -= 2;
        cpu->pc = 0x0000;
    } else {
        cpu_write(cpu, bus, cpu->sp - 2, cpu->pc & 0xFF);
        cpu->sp -= 2;
        interrupts_acknowledge(cpu->interrupts, pending);
        cpu->pc = handlers[pending];
    }

    return 20; // Interrupt handling takes 20 cycles
}
