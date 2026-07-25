#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "audio/audio_backend.h"
#include "audio/audio_backend_sdl.h"
#include "gameboy.h"
#include "input/input.h"
#include "input/input_tty.h"
#include "logger.h"
#include "renderer/renderer.h"
#include "renderer/renderer_ascii.h"

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
        LOG_ERROR("Usage: %s <rom> [screen-tty]", argv[0]);
        return -1;
    }

    const char *screen_tty = (argc >= 3) ? argv[2] : NULL;

    gameboy_t gb;
    if (gameboy_init(&gb, argv[1]) != 0) {
        return -1;
    }

    renderer_t renderer = renderer_ascii(screen_tty);
    renderer_init(&renderer);

    audio_backend_t audio_backend = audio_backend_sdl();
    audio_backend_init(&audio_backend);

    input_t input = input_tty(screen_tty);
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
                running = 0; // window closed or Escape pressed (SDL frontend only)
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
