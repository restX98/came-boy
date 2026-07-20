#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdbool.h>
#include <stdint.h>

#include "interrupts.h"

#define JOYP_SELECT_DPAD    0x10 // bit 4 drives D-Pad
#define JOYP_SELECT_BUTTONS 0x20 // bit 5 drives Buttons

typedef enum {
    // D-Pad
    JOYPAD_RIGHT,
    JOYPAD_LEFT,
    JOYPAD_UP,
    JOYPAD_DOWN,

    // Buttons
    JOYPAD_A,
    JOYPAD_B,
    JOYPAD_SELECT,
    JOYPAD_START,

    JOYPAD_KEY_COUNT,
} joypad_key_t;

typedef struct {
    uint8_t select_bits;   // only bits 4-5 meaningful; 0 = row selected

    uint8_t dpad_state;    // bit 0=Right, 1=Left, 2=Up,     3=Down
    uint8_t button_state;  // bit 0=A,     1=B,    2=Select, 3=Start
} joypad_reg_t;

void joypad_init(joypad_reg_t *jp);

// Called by io_reg for address $FF00
uint8_t joypad_read(joypad_reg_t *jp);
void joypad_write(joypad_reg_t *jp, uint8_t value);

void joypad_press(joypad_reg_t *jp, interrupt_regs_t *interrupts, joypad_key_t key);
void joypad_release(joypad_reg_t *jp, joypad_key_t key);

#endif // JOYPAD_H
