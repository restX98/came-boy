#include "oam_dma.h"

#include <assert.h>

#include "bus.h"

void oam_dma_init(oam_dma_reg_t *dma) {
    dma->active = false;
    dma->source = 0xFFFF;
    dma->index = 0;
    dma->start_delay = 0;
}

uint8_t oam_dma_read(oam_dma_reg_t *dma) {
    return dma->source >> 8;
}

void oam_dma_write(oam_dma_reg_t *dma, uint8_t value) {
    dma->active = true;
    dma->source = value << 8;
    dma->index = 0;
    dma->start_delay = 4;
}

void oam_dma_tick(bus_t *bus, int cycles) {
    oam_dma_reg_t *dma = &bus->io_reg.oam_dma;
    if (!dma->active) return;

    while (cycles > 0) {
        if (dma->start_delay > 0) {
            int consumed = (cycles < dma->start_delay) ? cycles : dma->start_delay;
            dma->start_delay -= consumed;
            cycles -= consumed;
            continue;
        }

        bus->oam.mem[dma->index] = bus_read(bus, dma->source + dma->index);
        dma->index++;
        cycles -= 4; // one byte per M-cycle

        if (dma->index >= OAM_SIZE) {
            dma->active = false;
            return;
        }
    }
}
