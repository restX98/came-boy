#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "bus.h"
#include "clock.h"
#include "cpu.h"
#include "memory/cartridge.h"
#include "ppu.h"

// The Game Boy: the top-level owner of the emulated machine. It owns the core
// components by value, constructs and wires them (bus -> cartridge, ppu ->
// lcd/vram/oam, clock -> ppu/bus), and advances them one instruction at a time.
//
// The components hold pointers into each other, so a gameboy_t must not be
// copied or moved after gameboy_init: the internal wiring points at these very
// fields.
typedef struct {
    cartridge_t cartridge;
    bus_t bus;
    cpu_t cpu;
    ppu_t ppu;
    gb_clock_t clock;
} gameboy_t;

// Load the cartridge at rom_path and wire the machine. Returns 0 on success,
// -1 on failure (on failure nothing needs to be freed).
int gameboy_init(gameboy_t *gb, const char *rom_path);

// Release everything gameboy_init acquired.
void gameboy_free(gameboy_t *gb);

// Run one CPU instruction and advance the rest of the machine by its t-cycles.
// Returns the cycle count, or -1 if the CPU has halted.
int gameboy_step(gameboy_t *gb);

#endif // GAMEBOY_H
