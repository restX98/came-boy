#include "joypad.h"

void joypad_init(joypad_reg_t *jp) {
    // Power-on: reads of $FF00 must return $CF = 1100 1111
    //   bits 6-7 = 11 -> unused, always 1
    //   bits 4-5 = 00 -> both rows selected (active-LOW)
    //   bits 0-3 = 1111 -> nothing pressed (active-LOW)
    jp->select_bits = 0x00;
    jp->dpad_state = 0x0F; // all released = all bits 1
    jp->button_state = 0x0F;
}

uint8_t joypad_read(joypad_reg_t *jp) {
    uint8_t nibble = 0x0F;

    if (!(jp->select_bits & JOYP_SELECT_DPAD))
        nibble &= jp->dpad_state;
    if (!(jp->select_bits & JOYP_SELECT_BUTTONS))
        nibble &= jp->button_state;

    return 0xC0 | jp->select_bits | nibble;
}

void joypad_write(joypad_reg_t *jp, uint8_t value) {
    // Only the select lines are writable; low nibble and bits 6-7 ignored.
    jp->select_bits = value & (JOYP_SELECT_DPAD | JOYP_SELECT_BUTTONS);
}

// --- Frontend side ---------------------------------------------------------
// Joypad interrupt (IF bit 4) fires on a high->low transition of a selected
// input line: i.e. a key goes down while its row is currently selected.
static bool press(joypad_reg_t *jp, uint8_t *row, joypad_key_t key, uint8_t select_mask) {
    bool was_released = (*row & key) != 0; // active-low: bit set = released
    *row &= (uint8_t)~key;                 // press = CLEAR the bit

    bool row_selected = !(jp->select_bits & select_mask);
    return row_selected && was_released;
}

bool joypad_press_dpad(joypad_reg_t *jp, joypad_key_t key) {
    return press(jp, &jp->dpad_state, key, JOYP_SELECT_DPAD);
}

bool joypad_press_button(joypad_reg_t *jp, joypad_key_t key) {
    return press(jp, &jp->button_state, key, JOYP_SELECT_BUTTONS);
}

void joypad_release_dpad(joypad_reg_t *jp, joypad_key_t key) {
    jp->dpad_state |= key;                 // release = SET the bit
}

void joypad_release_button(joypad_reg_t *jp, joypad_key_t key) {
    jp->button_state |= key;
}
