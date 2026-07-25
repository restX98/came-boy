#include "frontend/sdl_context.h"

#include "logger.h"
#include "ppu.h"

#define WINDOW_SCALE 10

static SDL_Window *g_window;
static SDL_Renderer *g_renderer;
static int g_refcount;

int sdl_context_acquire(SDL_Window **out_window, SDL_Renderer **out_renderer) {
    if (g_refcount == 0) {
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
            LOG_WARN("sdl_context: SDL_InitSubSystem(VIDEO) failed: %s", SDL_GetError());
            return -1;
        }

        g_window = SDL_CreateWindow(
            "came-boy",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            LCD_WIDTH * WINDOW_SCALE, LCD_HEIGHT * WINDOW_SCALE,
            SDL_WINDOW_SHOWN
        );
        if (!g_window) {
            LOG_WARN("sdl_context: SDL_CreateWindow failed: %s", SDL_GetError());
            return -1;
        }

        // No SDL_RENDERER_PRESENTVSYNC: the audio queue is already the
        // emulator's speed governor (see audio_backend_sdl.c) — a second,
        // independent ~60Hz vsync clock would drift against the Game Boy's
        // real 59.7275Hz rate and reintroduce the stutter that fix was for.
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
        if (!g_renderer) {
            LOG_WARN("sdl_context: SDL_CreateRenderer failed: %s", SDL_GetError());
            SDL_DestroyWindow(g_window);
            g_window = NULL;
            return -1;
        }
    }

    g_refcount++;
    *out_window = g_window;
    *out_renderer = g_renderer;
    return 0;
}

void sdl_context_release(void) {
    if (g_refcount == 0) return;

    g_refcount--;
    if (g_refcount == 0) {
        if (g_renderer) SDL_DestroyRenderer(g_renderer);
        if (g_window) SDL_DestroyWindow(g_window);
        g_renderer = NULL;
        g_window = NULL;
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

bool sdl_context_pump_events(joypad_reg_t *jp) {
    bool quit = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                if (e.key.repeat) break; // joypad_press is idempotent; skip OS auto-repeat noise

                joypad_key_t key;
                switch (e.key.keysym.sym) {
                    case SDLK_w:      key = JOYPAD_UP;     break;
                    case SDLK_s:      key = JOYPAD_DOWN;   break;
                    case SDLK_a:      key = JOYPAD_LEFT;   break;
                    case SDLK_d:      key = JOYPAD_RIGHT;  break;
                    case SDLK_p:      key = JOYPAD_A;      break;
                    case SDLK_l:      key = JOYPAD_B;      break;
                    case SDLK_v:      key = JOYPAD_SELECT; break;
                    case SDLK_b:      key = JOYPAD_START;  break;
                    case SDLK_ESCAPE:
                        if (e.type == SDL_KEYDOWN) quit = true;
                        continue;
                    default:
                        continue;
                }

                if (e.type == SDL_KEYDOWN) {
                    joypad_press(jp, key);
                } else {
                    joypad_release(jp, key);
                }
                break;
            }

            default:
                break;
        }
    }

    return quit;
}
