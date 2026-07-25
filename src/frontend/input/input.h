#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#include "io/joypad.h"

typedef struct input input_t;

struct input {
    int  (*init)(input_t *self);
    bool (*poll)(input_t *self, joypad_reg_t *jp);
    void (*deinit)(input_t *self);
    void *ctx;
};

int  input_init(input_t *in);
bool input_poll(input_t *in, joypad_reg_t *jp);
void input_deinit(input_t *in);

#endif // INPUT_H
