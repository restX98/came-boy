#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "bus.h"
#include "memory/cartridge.h"
#include "cpu.h"
#include "logger.h"

static volatile sig_atomic_t running = 1;

static void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

static void setup_signals(void) {
    struct sigaction sa = {
        .sa_handler = handle_sigint,
        .sa_flags = SA_RESETHAND,
    };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}

int main(int argc, char *argv[]) {
    setup_signals();

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

    while (running) {
        int cycles = cpu_step(&cpu, &bus);
        if (cycles < 0) {
            LOG_ERROR("CPU halted, exiting");
            break;
        }

        timer_tick(&bus.io_reg.timer, &bus.io_reg.interrupts, cycles);
    }


    if (!running) {
        LOG_INFO("Interrupted, shutting down");
    }

    // free resources
    cartridge_unload(&cartridge);
    bus_free(&bus);

    return 0;
}
