#include "input.h"

int input_init(input_t *in) {
    if (!in || !in->init) return 0;
    return in->init(in);
}

bool input_poll(input_t *in, joypad_reg_t *jp) {
    if (!in || !in->poll) return false;
    return in->poll(in, jp);
}

void input_deinit(input_t *in) {
    if (!in || !in->deinit) return;
    in->deinit(in);
}
