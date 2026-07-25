#include "audio/audio_backend.h"

int audio_backend_init(audio_backend_t *b) {
    if (!b || !b->init) return 0;
    return b->init(b);
}

void audio_backend_queue_sample(audio_backend_t *b, int16_t left, int16_t right) {
    if (!b || !b->queue_sample) return;
    b->queue_sample(b, left, right);
}

void audio_backend_deinit(audio_backend_t *b) {
    if (!b || !b->deinit) return;
    b->deinit(b);
}
