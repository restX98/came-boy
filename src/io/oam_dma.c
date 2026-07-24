#include "oam_dma.h"

#include <assert.h>

#include "bus.h"

void oam_dma_init(oam_dma_reg_t *dma) {
    dma->active = false;
    dma->source = 0xFFFF;
    dma->index = 0;
    dma->start_delay = 0;
    dma->pending = false;
    dma->pending_source = 0xFFFF;
    dma->pending_delay = 0;
}

uint8_t oam_dma_read(oam_dma_reg_t *dma) {
    return dma->source >> 8;
}

void oam_dma_write(oam_dma_reg_t *dma, uint8_t value) {
    if (dma->active && dma->start_delay == 0) {
        // A transfer is already copying
        dma->pending = true;
        dma->pending_source = value << 8;
        dma->pending_delay = 8;
        return;
    }

    // Fresh start
    dma->active = true;
    dma->source = value << 8;
    dma->index = 0;
    dma->start_delay = 8;
    dma->pending = false;
    dma->pending_delay = 0;
}

static uint8_t oam_dma_bus_read(bus_t *bus, uint16_t addr) {
    // DMG: on the DMA bus, WRAM is mirrored across $E000-$FFFF.
    if (addr >= 0xE000) {
        addr -= 0x2000;
    }
    return bus_read(bus, addr);
}

void oam_dma_tick(bus_t *bus, int cycles) {
    oam_dma_reg_t *dma = &bus->io_reg.oam_dma;
    if (!dma->active && !dma->pending) return;

    // Advance one M-cycle (4 t-cycles) at a time so the setup delay, the byte
    // transfers, and any pending-restart delay all step in lockstep.
    while (cycles >= 4 && (dma->active || dma->pending)) {
        cycles -= 4;

        if (dma->active) {
            if (dma->start_delay > 0) {
                // Setup delay: OAM is still accessible, no bytes copied yet.
                dma->start_delay -= 4;
            } else {
                bus->oam.mem[dma->index] = oam_dma_bus_read(bus, dma->source + dma->index);
                dma->index++;
                if (dma->index >= OAM_SIZE) {
                    dma->active = false;
                }
            }
        }

        // A restart requested mid-transfer counts its setup delay down here,
        // one M-cycle at a time, while the current transfer above keeps running
        // (which keeps OAM blocked). On expiry the new source takes over.
        if (dma->pending) {
            dma->pending_delay -= 4;
            if (dma->pending_delay == 0) {
                dma->source = dma->pending_source;
                dma->index = 0;
                dma->start_delay = 0;
                dma->active = true;
                dma->pending = false;
            }
        }
    }
}
