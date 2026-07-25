#include "frontend/input/input_sdl.h"

#include "frontend/sdl_context.h"

static int input_sdl_init(input_t *self) {
    (void)self;
    SDL_Window *window;
    SDL_Renderer *renderer;
    // Acquire (not create): ensures a window exists so keyboard events have
    // somewhere to be delivered, even if renderer_sdl init hasn't run yet.
    sdl_context_acquire(&window, &renderer);
    return 0;
}

static bool input_sdl_poll(input_t *self, joypad_reg_t *jp) {
    (void)self;
    return sdl_context_pump_events(jp);
}

static void input_sdl_deinit(input_t *self) {
    (void)self;
    sdl_context_release();
}

input_t input_sdl(void) {
    return (input_t) {
        .init = input_sdl_init,
        .poll = input_sdl_poll,
        .deinit = input_sdl_deinit,
        .ctx = NULL,
    };
}
