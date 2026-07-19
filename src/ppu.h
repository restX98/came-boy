#ifndef PPU_H
#define PPU_H

#include "bus.h"

#define LCD_WIDTH  160
#define LCD_HEIGHT 144

#define DOTS_PER_SCANLINE 456
#define SCANLINES_PER_FRAME 154
#define VBLANK_START_LINE 144

#define OAM_ENTRY_COUNT     40
#define MAX_SPRITES_PER_LINE 10

#define OAM_FLAG_PRIORITY 0x80  // 0 = sprite over BG, 1 = BG colors 1-3 over sprite
#define OAM_FLAG_Y_FLIP   0x40
#define OAM_FLAG_X_FLIP   0x20
#define OAM_FLAG_PALETTE  0x10  // 0 = OBP0, 1 = OBP1

typedef struct {
    uint16_t dot;
    uint8_t window_line;
    bool wy_condition;
    bool frame_ready;

    uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT];
} ppu_t;

void ppu_init(ppu_t *ppu);
void ppu_step(ppu_t *ppu, bus_t *bus, int t_cycles);

#endif // PPU_H
