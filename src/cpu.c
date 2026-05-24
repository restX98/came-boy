#include "cpu.h"

#include "logger.h"
#include "opcodes.h"

static int interrupt_service_routine(cpu_t *cpu, bus_t *bus, uint8_t pending);

void cpu_init(cpu_t *cpu) {
    LOG_INFO("Initializing CPU");

    cpu->af.hi = 0x01;
    cpu->af.lo = 0xB0; // Z=1, N=0, H=1, C=1
    cpu->bc.reg = 0x0013;
    cpu->de.reg = 0x00D8;
    cpu->hl.reg = 0x014D;
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0100;
    cpu->ime.enabled = false;
    cpu->ime.scheduled = false;

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
    uint8_t ie = bus->io_reg.interrupts.enable.reg;
    uint8_t if_ = bus->io_reg.interrupts.flag.reg;
    uint8_t pending = ie & if_ & 0b00011111;

    if (pending && cpu->ime.enabled) {
        LOG_DEBUG("Pending IRQs: 0x%02X (IE=0x%02X IF=0x%02X)", pending, ie, if_);
        return interrupt_service_routine(cpu, bus, pending);
    }

    if (cpu->halted) {
        if (pending) {
            // Pending IRQs wake the CPU from HALT even when IME=0.
            LOG_DEBUG("Waking from HALT (IME=0): pending=0x%02X", pending);
            cpu->halted = false;
        }
        return 4; // Waking from HALT takes 4 cycles
    }

    // EI delays enabling IME by one instruction. It sets ime_scheduled, and we
    // promote it to ime here, at the START of the following step, before executing
    // the next instruction. This guarantees that instruction runs uninterrupted,
    // and interrupts can only fire from the step after that.
    if (cpu->ime.scheduled) {
        cpu->ime.enabled = true;
        cpu->ime.scheduled = false;
    }

    uint8_t instruction = bus_read(bus, cpu->pc);
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

static int interrupt_service_routine(cpu_t *cpu, bus_t *bus, uint8_t pending) {
    // Find lowest set bit (highest priority): 0=VBlank, 1=LCD, 2=Timer, 3=Serial, 4=Joypad.
    int bit = __builtin_ctz(pending);
    static const uint16_t handlers[] = { 0x40, 0x48, 0x50, 0x58, 0x60 };

    // Dispatching an interrupt always wakes the CPU from HALT.
    cpu->halted = false;

    bus->io_reg.interrupts.flag.reg &= ~(1 << bit); // Clear IF for this interrupt
    cpu->ime.enabled = false;

    bus_write(bus, cpu->sp - 1, cpu->pc >> 8);
    bus_write(bus, cpu->sp - 2, cpu->pc & 0xFF);
    cpu->sp -= 2;
    cpu->pc = handlers[bit];

    return 20; // Interrupt handling takes 20 cycles
}
