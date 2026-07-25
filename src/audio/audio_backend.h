#ifndef AUDIO_BACKEND_H
#define AUDIO_BACKEND_H

#include <stdint.h>

// Generic audio output backend, mirroring renderer_t (src/renderer/renderer.h):
// the emulator core only knows this abstraction and hands it one stereo
// sample at a time as the APU produces them. Each backend decides how (or
// whether) those samples reach real speakers.
typedef struct audio_backend audio_backend_t;

struct audio_backend {
    int  (*init)(audio_backend_t *self);
    void (*queue_sample)(audio_backend_t *self, int16_t left, int16_t right);
    void (*deinit)(audio_backend_t *self);
    void *ctx;  // backend-private state
};

int  audio_backend_init(audio_backend_t *b);
void audio_backend_queue_sample(audio_backend_t *b, int16_t left, int16_t right);
void audio_backend_deinit(audio_backend_t *b);

#endif // AUDIO_BACKEND_H
