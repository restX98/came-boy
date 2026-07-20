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

typedef struct {
    uint8_t *row;
    uint8_t bit;
    uint8_t select_mask;
} key_loc_t;

static key_loc_t resolve_key(joypad_reg_t *jp, joypad_key_t key) {
    if (key <= JOYPAD_DOWN)
        return (key_loc_t) { &jp->dpad_state, (uint8_t)(1u << key), JOYP_SELECT_DPAD };
    return (key_loc_t) { &jp->button_state, (uint8_t)(1u << (key - JOYPAD_A)), JOYP_SELECT_BUTTONS };
}

// Joypad interrupt (IF bit 4) fires on a high->low transition of a selected
// input line: i.e. a key goes down while its row is currently selected.
void joypad_press(joypad_reg_t *jp, interrupt_regs_t *interrupts, joypad_key_t key) {
    key_loc_t loc = resolve_key(jp, key);

    bool was_released = (*loc.row & loc.bit) != 0; // active-low: bit set = released
    *loc.row &= (uint8_t)~loc.bit;                 // press = CLEAR the bit

    bool row_selected = !(jp->select_bits & loc.select_mask);
    if (row_selected && was_released)
        interrupts_request(interrupts, INT_JOYPAD);
}

void joypad_release(joypad_reg_t *jp, joypad_key_t key) {
    key_loc_t loc = resolve_key(jp, key);
    *loc.row |= loc.bit;
}
