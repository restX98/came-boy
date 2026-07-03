#include "timer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void timer_init(timer_regs_t *timer) {
    timer->div_counter = 0xABD4;
    timer->tima = 0x00;
    timer->tma = 0x00;
    timer->tac.reg = 0xF8;

    timer->tima_reload_pending = 0;
    timer->tima_just_reloaded = false;
}

void timer_tick(timer_regs_t *timer, interrupt_regs_t *interrupts, int cycles) {
    uint16_t old_div = timer->div_counter;
    timer->div_counter += cycles;

    timer->tima_just_reloaded = false;

    // Handle any pending TIMA reload from a previous overflow.
    if (timer->tima_reload_pending > 0) {
        if (cycles >= timer->tima_reload_pending) {
            timer->tima = timer->tma;
            interrupts_request(interrupts, INT_TIMER);
            timer->tima_reload_pending = 0;
            timer->tima_just_reloaded = true;
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
    static const uint16_t freq_table[4] = { 1024, 16, 64, 256 };
    uint16_t freq = freq_table[timer->tac.clock_select];

    uint16_t ticks = (timer->div_counter / freq) - (old_div / freq);
    while (ticks--) {
        if (timer->tima == 0xFF) {
            timer->tima = 0x00;
            timer->tima_reload_pending = 4;
            break;
        }
        timer->tima++;
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

static bool timer_and_gate(timer_regs_t *timer, uint8_t clock_select) {
    static const uint8_t bit_table[4] = { 9, 3, 5, 7 };
    bool div_bit = (timer->div_counter >> bit_table[clock_select]) & 1;
    return timer->tac.enable && div_bit;
}

static void timer_apply_falling_edge(timer_regs_t *timer, bool and_before, bool and_after) {
    if (and_before && !and_after) {
        timer->tima++;
        if (timer->tima == 0x00) {
            timer->tima_reload_pending = 4;
        }
    }
}

void timer_write(timer_regs_t *timer, uint16_t addr, uint8_t value) {
    if (addr == 0xFF04) {
        uint8_t old_cs = timer->tac.clock_select;
        bool and_before = timer_and_gate(timer, old_cs);

        // Writing any value to this register resets it to $00.
        timer->div_counter = 0;

        bool and_after = timer_and_gate(timer, old_cs); // same clock_select, div changed
        timer_apply_falling_edge(timer, and_before, and_after);
    } else if (addr == 0xFF05) {
        if (!timer->tima_just_reloaded) {
            timer->tima = value;
            timer->tima_reload_pending = 0;
        }
    } else if (addr == 0xFF06) {
        timer->tma = value;

        if (timer->tima_just_reloaded) {
            timer->tima = value;
        }
    } else if (addr == 0xFF07) {
        uint8_t old_cs = timer->tac.clock_select;
        bool and_before = timer_and_gate(timer, old_cs);

        timer->tac.reg = value;

        bool and_after = timer_and_gate(timer, old_cs); // old clock_select, enable may have changed
        timer_apply_falling_edge(timer, and_before, and_after);
    } else {
        assert(0 && "Unhandled timer address");
    }
}
