#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "io_reg.h"

// ---- Mock functions ----

typedef struct {
    interrupt_regs_t *interrupts;
} interrupts_init_call_t;

typedef struct {
    size_t call_count;
    interrupts_init_call_t calls[10];
} interrupts_init_stats_t;

static interrupts_init_stats_t interrupts_init_stats;

void interrupts_init(interrupt_regs_t *interrupts) {
    if (interrupts_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_init_stats");
    }

    interrupts_init_call_t *call = &interrupts_init_stats.calls[interrupts_init_stats.call_count];
    call->interrupts = interrupts;

    interrupts_init_stats.call_count++;
}

typedef struct {
    interrupt_regs_t *interrupts;
    uint16_t addr;
    uint8_t return_value;
} interrupts_read_call_t;

typedef struct {
    size_t call_count;
    interrupts_read_call_t calls[10];
} interrupts_read_stats_t;

static interrupts_read_stats_t interrupts_read_stats;

uint8_t interrupts_read(interrupt_regs_t *interrupts, uint16_t addr) {
    if (interrupts_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_read_stats");
    }

    interrupts_read_call_t *call = &interrupts_read_stats.calls[interrupts_read_stats.call_count];
    call->interrupts = interrupts;
    call->addr = addr;

    interrupts_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    interrupt_regs_t *interrupts;
    uint16_t addr;
    uint8_t value;
} interrupts_write_call_t;

typedef struct {
    size_t call_count;
    interrupts_write_call_t calls[10];
} interrupts_write_stats_t;

static interrupts_write_stats_t interrupts_write_stats;

void interrupts_write(interrupt_regs_t *interrupts, uint16_t addr, uint8_t value) {
    if (interrupts_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_write_stats");
    }

    interrupts_write_call_t *call = &interrupts_write_stats.calls[interrupts_write_stats.call_count];
    call->interrupts = interrupts;
    call->addr = addr;
    call->value = value;

    interrupts_write_stats.call_count++;
}

typedef struct {
    timer_regs_t *timer;
} timer_init_call_t;

typedef struct {
    size_t call_count;
    timer_init_call_t calls[10];
} timer_init_stats_t;

static timer_init_stats_t timer_init_stats;

void timer_init(timer_regs_t *timer) {
    if (timer_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for timer_init_stats");
    }

    timer_init_call_t *call = &timer_init_stats.calls[timer_init_stats.call_count];
    call->timer = timer;

    timer_init_stats.call_count++;
}

typedef struct {
    timer_regs_t *timer;
    uint16_t addr;
    uint8_t return_value;
} timer_read_call_t;

typedef struct {
    size_t call_count;
    timer_read_call_t calls[10];
} timer_read_stats_t;

static timer_read_stats_t timer_read_stats;

uint8_t timer_read(timer_regs_t *timer, uint16_t addr) {
    if (timer_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for timer_read_stats");
    }

    timer_read_call_t *call = &timer_read_stats.calls[timer_read_stats.call_count];
    call->timer = timer;
    call->addr = addr;

    timer_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    timer_regs_t *timer;
    uint16_t addr;
    uint8_t value;
} timer_write_call_t;

typedef struct {
    size_t call_count;
    timer_write_call_t calls[10];
} timer_write_stats_t;

static timer_write_stats_t timer_write_stats;

void timer_write(timer_regs_t *timer, uint16_t addr, uint8_t value) {
    if (timer_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for timer_write_stats");
    }

    timer_write_call_t *call = &timer_write_stats.calls[timer_write_stats.call_count];
    call->timer = timer;
    call->addr = addr;
    call->value = value;

    timer_write_stats.call_count++;
}

typedef struct {
    joypad_reg_t *joyp;
} joypad_init_call_t;

typedef struct {
    size_t call_count;
    joypad_init_call_t calls[10];
} joypad_init_stats_t;

static joypad_init_stats_t joypad_init_stats;

void joypad_init(joypad_reg_t *joyp) {
    if (joypad_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for joypad_init_stats");
    }

    joypad_init_call_t *call = &joypad_init_stats.calls[joypad_init_stats.call_count];
    call->joyp = joyp;

    joypad_init_stats.call_count++;
}

typedef struct {
    joypad_reg_t *joyp;
    uint8_t return_value;
} joypad_read_call_t;

typedef struct {
    size_t call_count;
    joypad_read_call_t calls[10];
} joypad_read_stats_t;

static joypad_read_stats_t joypad_read_stats;

uint8_t joypad_read(joypad_reg_t *joyp) {
    if (joypad_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for joypad_read_stats");
    }

    joypad_read_call_t *call = &joypad_read_stats.calls[joypad_read_stats.call_count];
    call->joyp = joyp;

    joypad_read_stats.call_count++;

    return call->return_value;
}

typedef struct {
    joypad_reg_t *joyp;
    uint8_t value;
} joypad_write_call_t;

typedef struct {
    size_t call_count;
    joypad_write_call_t calls[10];
} joypad_write_stats_t;

static joypad_write_stats_t joypad_write_stats;

void joypad_write(joypad_reg_t *joyp, uint8_t value) {
    if (joypad_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for joypad_write_stats");
    }

    joypad_write_call_t *call = &joypad_write_stats.calls[joypad_write_stats.call_count];
    call->joyp = joyp;
    call->value = value;

    joypad_write_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    interrupts_init_stats = (interrupts_init_stats_t){ 0 };
    interrupts_read_stats = (interrupts_read_stats_t){ 0 };
    interrupts_write_stats = (interrupts_write_stats_t){ 0 };
    timer_init_stats = (timer_init_stats_t){ 0 };
    timer_read_stats = (timer_read_stats_t){ 0 };
    timer_write_stats = (timer_write_stats_t){ 0 };
    joypad_init_stats = (joypad_init_stats_t){ 0 };
    joypad_read_stats = (joypad_read_stats_t){ 0 };
    joypad_write_stats = (joypad_write_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

int main(void) {
    UNITY_BEGIN();

    return UNITY_END();
}
