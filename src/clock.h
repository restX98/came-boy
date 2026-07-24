#ifndef CLOCK_H
#define CLOCK_H

#include "bus.h"
#include "ppu.h"

// The system clock: the set of components that advance with time. It fans a
// t-cycle count out to every timed peripheral so callers advance the whole
// machine in one call instead of poking each peripheral by hand.
//
// Named `gb_clock_t` rather than `clock_t`, which is a standard <time.h> type
// (pulled in transitively via bus.h -> cartridge.h).
typedef struct {
    ppu_t *ppu;
    bus_t *bus; // provides the OAM DMA engine and the timer (bus->io_reg)
} gb_clock_t;

void clock_init(gb_clock_t *clock, ppu_t *ppu, bus_t *bus);
void clock_tick(gb_clock_t *clock, int cycles);

#endif // CLOCK_H
