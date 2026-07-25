#ifndef FRONTEND_SDL_CONTEXT_H
#define FRONTEND_SDL_CONTEXT_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "io/joypad.h"

// Shared SDL window + renderer, used by both renderer_sdl
// (src/renderer/renderer_sdl.c) and input_sdl (src/input/input_sdl.c).
// Reference-counted so either backend can be constructed/destroyed
// independently and in either order — the window is only created on the
// first acquire and only torn down once every acquirer has released it.
//
// SDL delivers all window and keyboard events through one process-wide
// queue, so exactly one place may call SDL_PollEvent: sdl_context_pump_events
// below, called from input_sdl's poll().

// Acquires the shared window/renderer, creating them on first use. Returns 0
// on success (with *out_window/*out_renderer populated) or -1 if no display
// is available.
int sdl_context_acquire(SDL_Window **out_window, SDL_Renderer **out_renderer);
void sdl_context_release(void);

// Drains pending SDL events: keypresses update `jp`; a window close or
// Escape sets the returned quit flag.
bool sdl_context_pump_events(joypad_reg_t *jp);

#endif // FRONTEND_SDL_CONTEXT_H
