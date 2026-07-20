#ifndef INPUT_H
#define INPUT_H

#include "io/joypad.h"

typedef struct input input_t;

struct input {
    int  (*init)(input_t *self);
    void (*poll)(input_t *self, joypad_reg_t *jp, interrupt_regs_t *interrupts);
    void (*deinit)(input_t *self);
    void *ctx;
};

int  input_init(input_t *in);
void input_poll(input_t *in, joypad_reg_t *jp, interrupt_regs_t *interrupts);
void input_deinit(input_t *in);

#endif // INPUT_H
