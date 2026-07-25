#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

// Generic renderer interface.
//
// The emulator core only knows about this abstraction: it hands a completed
// framebuffer to renderer_render() once per frame and never cares which backend
// is drawing. Each backend (ASCII/TTY today, a graphics engine later) fills in
// the function pointers below and stashes its private state in `ctx`.
typedef struct renderer renderer_t;

struct renderer {
    int  (*init)(renderer_t *self);
    void (*render)(renderer_t *self, const uint8_t *framebuffer);
    void (*deinit)(renderer_t *self);
    void *ctx;  // backend-private state
};

// Thin dispatchers so callers don't poke the function pointers directly and
// tolerate backends that leave optional hooks NULL.
int  renderer_init(renderer_t *r);
void renderer_render(renderer_t *r, const uint8_t *framebuffer);
void renderer_deinit(renderer_t *r);

#endif // RENDERER_H
