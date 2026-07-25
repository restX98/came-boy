#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "frontend/audio/audio_backend.h"
#include "frontend/audio/audio_backend_sdl.h"
#include "frontend/input/input.h"
#include "frontend/input/input_sdl.h"
#include "frontend/input/input_tty.h"
#include "frontend/renderer/renderer.h"
#include "frontend/renderer/renderer_ascii.h"
#include "frontend/renderer/renderer_sdl.h"
#include "gameboy.h"
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

    const char *usage = "Usage: %s <rom> [--tty[=<path>]]";

    if (argc < 2) {
        LOG_ERROR(usage, argv[0]);
        return -1;
    }

    // SDL is the default frontend; --tty opts into the ASCII/terminal one,
    // optionally pinning it to a specific tty (bare --tty falls back to
    // stdout/stdin, same as passing no path did before this flag existed).
    bool use_ascii_frontend = false;
    const char *screen_tty = NULL;

    if (argc >= 3) {
        const char *arg = argv[2];
        if (strncmp(arg, "--tty", 5) != 0 || (arg[5] != '\0' && arg[5] != '=')) {
            LOG_ERROR(usage, argv[0]);
            return -1;
        }
        use_ascii_frontend = true;
        if (arg[5] == '=' && arg[6] != '\0') {
            screen_tty = arg + 6;
        }
    }

    gameboy_t gb;
    if (gameboy_init(&gb, argv[1]) != 0) {
        return -1;
    }

    renderer_t renderer = use_ascii_frontend ? renderer_ascii(screen_tty) : renderer_sdl();
    renderer_init(&renderer);

    audio_backend_t audio_backend = audio_backend_sdl();
    audio_backend_init(&audio_backend);

    input_t input = use_ascii_frontend ? input_tty(screen_tty) : input_sdl();
    input_init(&input);

    while (running) {
        if (gameboy_step(&gb) < 0) {
            LOG_ERROR("CPU halted, exiting");
            break;
        }

        if (gb.bus.io_reg.audio.sample_ready) {
            audio_backend_queue_sample(
                &audio_backend,
                gb.bus.io_reg.audio.sample_left,
                gb.bus.io_reg.audio.sample_right
            );
            gb.bus.io_reg.audio.sample_ready = false;
        }

        if (gb.ppu.frame_ready) {
            if (input_poll(&input, &gb.bus.io_reg.joyp)) {
                running = 0; // window closed or Escape pressed
            }
            renderer_render(&renderer, gb.ppu.framebuffer);
            gb.ppu.frame_ready = false;
        }
    }

    input_deinit(&input);
    audio_backend_deinit(&audio_backend);
    renderer_deinit(&renderer);
    gameboy_free(&gb);

    return 0;
}
