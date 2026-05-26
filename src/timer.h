#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "interrupts.h"

typedef union {
    uint8_t reg;
    struct {
        uint8_t clock_select : 2; // bits 0-1 — input clock select
        uint8_t enable : 1;       // bit 2    — timer enable
        uint8_t _ : 5;            // bits 3-7, unused, always 1 on real hardware
    };
} timer_ctrl_t;

typedef struct {
    // DIV is incremented at 16384 Hz on DMG (4194304 Hz / 256 = 16384 Hz), i.e. once
    // every 256 CPU cycles. We model this with a 16-bit internal counter that ticks
    // every cycle; DIV is simply the upper 8 bits, so it naturally increments once
    // per 256 cycles via the low byte's overflow into the high byte.
    union {
        uint16_t div_counter;
        struct {
            uint8_t _;
            uint8_t div; // 0xFF04 — Divider register
        };
    };
    uint8_t tima;       // 0xFF05 — Timer counter
    uint8_t tma;        // 0xFF06 — Timer modulo
    timer_ctrl_t tac;   // 0xFF07 — Timer control

    uint16_t tima_counter; // internal counter for TIMA increments
    uint8_t tima_reload_pending; // pending reload for TIMA
} timer_regs_t;

void timer_init(timer_regs_t *timer);
void timer_tick(timer_regs_t *timer, interrupt_regs_t *interrupts, int cycles);

uint8_t timer_read(timer_regs_t *timer, uint16_t addr);
void timer_write(timer_regs_t *timer, uint16_t addr, uint8_t value);

#endif // TIMER_H
