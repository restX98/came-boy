#include <stdio.h>
#include <unistd.h>

#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "logger.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        LOG_ERROR("Usage: %s <rom>", argv[0]);
        return -1;
    }

    cartridge_t cartridge = { 0 };
    bus_t bus = { 0 };
    cpu_t cpu = { 0 };

    if (cartridge_load(&cartridge, argv[1]) != 0) {
        LOG_ERROR("Could not load cartridge");
        return -1;
    }

    if (bus_init(&bus, &cartridge) != 0) {
        cartridge_unload(&cartridge);
        LOG_ERROR("Could not initialize bus");
        return -1;
    }

    cpu_init(&cpu);

    while (1) {
        int cycles = cpu_step(&cpu, &bus);
        if (cycles < 0) {
            LOG_ERROR("CPU halted, exiting");
            break;
        }

        timer_tick(&bus.io_reg.timer, &bus.io_reg.interrupts, cycles);
    }

    // free resources
    cartridge_unload(&cartridge);
    bus_free(&bus);

    return 0;
}
