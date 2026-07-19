#ifndef OAM_DMA_H
#define OAM_DMA_H

#include <stdbool.h>
#include <stdint.h>

// Forward declaration to avoid a circular include with bus.h
// (bus.h -> io_reg.h -> oam_dma.h). The full definition is only needed
// in oam_dma.c, which includes bus.h directly.
typedef struct bus_s bus_t;

typedef struct {
    bool active;
    uint16_t source;
    uint8_t index;
    uint8_t start_delay;
} oam_dma_reg_t;

void oam_dma_init(oam_dma_reg_t *dma);

uint8_t oam_dma_read(oam_dma_reg_t *dma);
void oam_dma_write(oam_dma_reg_t *dma, uint8_t value);

void oam_dma_tick(bus_t *bus, int cycles);

#endif // OAM_DMA_H
