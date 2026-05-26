#include "timer.h"

#include <assert.h>

void timer_init(timer_regs_t *timer) {
    timer->div_counter = 0xAB00;
    timer->tima = 0x00;
    timer->tma = 0x00;
    timer->tac.reg = 0xF8;

    timer->tima_counter = 0;
    timer->tima_reload_pending = 0;
}

void timer_tick(timer_regs_t *timer, interrupt_regs_t *interrupts, int cycles) {
    timer->div_counter += cycles;

    // Handle any pending TIMA reload from a previous overflow.
    if (timer->tima_reload_pending > 0) {
        if (cycles >= timer->tima_reload_pending) {
            // TIMA overflowed, reset to TMA and request timer interrupt
            timer->tima = timer->tma;
            interrupts_request(interrupts, INT_TIMER);
            timer->tima_reload_pending = 0;
        } else {
            timer->tima_reload_pending -= cycles;
        }
    }

    if (!timer->tac.enable) {
        return;
    }

    // The timer frequency is determined by the clock select bits in TAC:
    // 00: 4096 Hz (increments every 1024 cycles)
    // 01: 262144 Hz (increments every 16 cycles)
    // 10: 65536 Hz (increments every 64 cycles)
    // 11: 16384 Hz (increments every 256 cycles)
    static const int freq_cycles_table[4] = { 1024, 16, 64, 256 };
    int freq_cycles = freq_cycles_table[timer->tac.clock_select];

    timer->tima_counter += cycles;
    while (timer->tima_counter >= freq_cycles) {
        timer->tima_counter -= freq_cycles;
        timer->tima++;
        if (timer->tima == 0x00) {
            timer->tima_reload_pending = 4;
        }
    }
}

uint8_t timer_read(timer_regs_t *timer, uint16_t addr) {
    if (addr == 0xFF04) {
        return timer->div;
    } else if (addr == 0xFF05) {
        return timer->tima;
    } else if (addr == 0xFF06) {
        return timer->tma;
    } else if (addr == 0xFF07) {
        return timer->tac.reg;
    }

    assert(0 && "Unhandled timer address");
    return 0xFF;
}

void timer_write(timer_regs_t *timer, uint16_t addr, uint8_t value) {
    if (addr == 0xFF04) {
        // Writing any value to this register resets it to $00.
        timer->div_counter = 0;
    } else if (addr == 0xFF05) {
        timer->tima = value;
    } else if (addr == 0xFF06) {
        timer->tma = value;
    } else if (addr == 0xFF07) {
        timer->tac.reg = value;
    } else {
        assert(0 && "Unhandled timer address");
    }
}
