#include "interrupts.h"

#include <assert.h>

void interrupts_init(interrupt_regs_t *interrupts) {
    interrupts->flag.reg = 0xE1;
    interrupts->enable.reg = 0x00;
}

void interrupts_request(interrupt_regs_t *interrupts, interrupt_t interrupt) {
    interrupts->flag.reg |= (1 << interrupt);
}

void interrupts_acknowledge(interrupt_regs_t *interrupts, interrupt_t interrupt) {
    interrupts->flag.reg &= ~(1 << interrupt);
}

int interrupts_pending(const interrupt_regs_t *interrupts) {
    uint8_t pending = interrupts->enable.reg & interrupts->flag.reg & 0b00011111;
    if (!pending) return -1;
    return __builtin_ctz(pending);  // lowest set bit = highest priority
}

uint8_t interrupts_read(interrupt_regs_t *interrupts, uint16_t addr) {
    if (addr == 0xFF0F) {
        return interrupts->flag.reg;
    } else if (addr == 0xFFFF) {
        return interrupts->enable.reg;
    }

    assert(0 && "Unhandled interrupts address");
    return 0xFF;
}

void interrupts_write(interrupt_regs_t *interrupts, uint16_t addr, uint8_t value) {
    if (addr == 0xFF0F) {
        interrupts->flag.reg = value | 0b11100000;
    } else if (addr == 0xFFFF) {
        interrupts->enable.reg = value;
    } else {
        assert(0 && "Unhandled interrupts address");
    }
}
