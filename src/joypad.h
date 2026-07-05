#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdbool.h>
#include <stdint.h>

#define JOYP_SELECT_DPAD    0x10 // bit 4 drives D-Pad
#define JOYP_SELECT_BUTTONS 0x20 // bit 5 drives Buttons

typedef enum {
    // D-Pad 
    JOYPAD_RIGHT = 0x01,
    JOYPAD_LEFT = 0x02,
    JOYPAD_UP = 0x04,
    JOYPAD_DOWN = 0x08,

    // Buttons
    JOYPAD_A = 0x01,
    JOYPAD_B = 0x02,
    JOYPAD_SELECT = 0x04,
    JOYPAD_START = 0x08,
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

// Frontend interface - called by input layer.
bool joypad_press_dpad(joypad_reg_t *jp, joypad_key_t key);
bool joypad_press_button(joypad_reg_t *jp, joypad_key_t key);
void joypad_release_dpad(joypad_reg_t *jp, joypad_key_t key);
void joypad_release_button(joypad_reg_t *jp, joypad_key_t key);

#endif // JOYPAD_H
