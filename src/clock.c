#include "clock.h"

#include "io/audio.h"
#include "io/oam_dma.h"
#include "io/timer.h"

void clock_init(gb_clock_t *clock, ppu_t *ppu, bus_t *bus) {
    clock->ppu = ppu;
    clock->bus = bus;
}

void clock_tick(gb_clock_t *clock, int cycles) {
    ppu_step(clock->ppu, cycles);
    oam_dma_tick(clock->bus, cycles);
    timer_tick(&clock->bus->io_reg.timer, cycles);
    audio_tick(&clock->bus->io_reg.audio, cycles, clock->bus->io_reg.timer.div, clock->bus->io_reg.wp_ram);
}
