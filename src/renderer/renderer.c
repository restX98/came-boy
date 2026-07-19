#include "renderer/renderer.h"

int renderer_init(renderer_t *r) {
    if (!r || !r->init) return 0;
    return r->init(r);
}

void renderer_render(renderer_t *r, const uint8_t *framebuffer) {
    if (!r || !r->render) return;
    r->render(r, framebuffer);
}

void renderer_deinit(renderer_t *r) {
    if (!r || !r->deinit) return;
    r->deinit(r);
}
