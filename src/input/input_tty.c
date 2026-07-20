#include "input/input_tty.h"

#include <fcntl.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#include "logger.h"

#define RELEASE_MS 120

typedef struct {
    const char *tty_path;
    int fd;
    bool raw_active;  // whether orig_termios holds a valid saved state to restore
    struct termios orig_termios;
    long last_seen[JOYPAD_KEY_COUNT];  // indexed by joypad_key_t
} tty_ctx_t;

static long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static int input_tty_init(input_t *self) {
    tty_ctx_t *ctx = self->ctx;

    ctx->fd = -1;
    if (ctx->tty_path) {
        ctx->fd = open(ctx->tty_path, O_RDONLY | O_NONBLOCK);
        if (ctx->fd < 0) {
            LOG_WARN("Could not open %s for input, falling back to stdin", ctx->tty_path);
        }
    }
    if (ctx->fd < 0) ctx->fd = STDIN_FILENO;

    // Only touch the line discipline if this really is a terminal and we can
    // read its current state; otherwise orig_termios would hold garbage and
    // deinit() would restore that garbage onto the fd, corrupting the tty.
    ctx->raw_active = false;
    if (isatty(ctx->fd) && tcgetattr(ctx->fd, &ctx->orig_termios) == 0) {
        struct termios raw = ctx->orig_termios;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(ctx->fd, TCSANOW, &raw) == 0) {
            ctx->raw_active = true;
        }
    }

    int flags = fcntl(ctx->fd, F_GETFL, 0);
    fcntl(ctx->fd, F_SETFL, flags | O_NONBLOCK);

    return 0;
}

static void input_tty_poll(input_t *self, joypad_reg_t *jp, interrupt_regs_t *interrupts) {
    tty_ctx_t *ctx = self->ctx;
    long t = now_ms();

    char c;
    while (read(ctx->fd, &c, 1) == 1) {
        joypad_key_t key;
        switch (c) {
            case 'w': key = JOYPAD_UP;     break;
            case 's': key = JOYPAD_DOWN;   break;
            case 'a': key = JOYPAD_LEFT;   break;
            case 'd': key = JOYPAD_RIGHT;  break;
            case 'l': key = JOYPAD_A;      break;
            case 'p': key = JOYPAD_B;      break;
            case 'v': key = JOYPAD_SELECT; break;
            case 'b': key = JOYPAD_START;  break;
            default: continue;
        }
        joypad_press(jp, interrupts, key);
        ctx->last_seen[key] = t;
    }

    // No auto-repeat on a raw tty, so synthesise a release once a key has been
    // quiet for RELEASE_MS.
    for (joypad_key_t key = 0; key < JOYPAD_KEY_COUNT; key++) {
        if (t - ctx->last_seen[key] > RELEASE_MS) joypad_release(jp, key);
    }
}

static void input_tty_deinit(input_t *self) {
    tty_ctx_t *ctx = self->ctx;
    if (ctx->raw_active) tcsetattr(ctx->fd, TCSANOW, &ctx->orig_termios);
    if (ctx->fd != STDIN_FILENO) close(ctx->fd);
}

static tty_ctx_t g_tty_ctx;

input_t input_tty(const char *tty_path) {
    g_tty_ctx = (tty_ctx_t){ .tty_path = tty_path, .fd = -1 };
    return (input_t) {
        .init = input_tty_init,
            .poll = input_tty_poll,
            .deinit = input_tty_deinit,
            .ctx = &g_tty_ctx,
    };
}
