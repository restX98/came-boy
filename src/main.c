#include <stdio.h>
#include <unistd.h>

#include "logger.h"
#include "cartridge.h"
#include "bus.h"
#include "cpu.h"

int main(void) {

    cartridge_t cartridge = { 0 };
    bus_t bus = { 0 };
    cpu_t cpu = { 0 };

    if (cartridge_load(&cartridge, "./roms/Tetris.gb") != 0) {
        LOG_ERROR("Could not load cartridge");
        return -1;
    }

    bus_init(&bus, &cartridge);
    cpu_init(&cpu);

    while (1) {
        int cycles = cpu_step(&cpu, &bus);
        if (cycles < 0) {
            LOG_ERROR("CPU halted, exiting");
            break;
        }
    }

    // free resources
    cartridge_unload(&cartridge);

    return 0;
}
