#ifndef PPU_H
#define PPU_H

#include "bus.h"

#define LCD_WIDTH  160
#define LCD_HEIGHT 144

#define DOTS_PER_SCANLINE 456
#define SCANLINES_PER_FRAME 154
#define VBLANK_START_LINE 144

typedef struct {
    uint16_t dot;
    bool frame_ready;

    uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT];
} ppu_t;

void ppu_init(ppu_t *ppu);
void ppu_step(ppu_t *ppu, bus_t *bus, int t_cycles);

#endif // PPU_H
