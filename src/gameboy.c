#include "gameboy.h"

#include "logger.h"

int gameboy_init(gameboy_t *gb, const char *rom_path) {
    *gb = (gameboy_t){ 0 };

    if (cartridge_load(&gb->cartridge, rom_path) != 0) {
        LOG_ERROR("Could not load cartridge");
        return -1;
    }

    if (bus_init(&gb->bus, &gb->cartridge) != 0) {
        LOG_ERROR("Could not initialize bus");
        cartridge_unload(&gb->cartridge);
        return -1;
    }

    ppu_init(&gb->ppu, &gb->bus.io_reg.lcd, &gb->bus.vram, &gb->bus.oam);
    clock_init(&gb->clock, &gb->ppu, &gb->bus);
    cpu_init(&gb->cpu, &gb->bus.io_reg.interrupts, &gb->clock);

    return 0;
}

void gameboy_free(gameboy_t *gb) {
    cartridge_unload(&gb->cartridge);
    bus_free(&gb->bus);
}

int gameboy_step(gameboy_t *gb) {
    // cpu_step advances the whole machine by one instruction: it drives the clock
    // for every M-cycle (memory accesses, internal cycles, and the remainder).
    return cpu_step(&gb->cpu, &gb->bus);
}
