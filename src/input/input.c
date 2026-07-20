#include "input.h"

int input_init(input_t *in) {
    if (!in || !in->init) return 0;
    return in->init(in);
}

void input_poll(input_t *in, joypad_reg_t *jp, interrupt_regs_t *interrupts) {
    if (!in || !in->poll) return;
    in->poll(in, jp, interrupts);
}

void input_deinit(input_t *in) {
    if (!in || !in->deinit) return;
    in->deinit(in);
}
