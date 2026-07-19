#include "unity.h"
#include "log_helpers.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "renderer/renderer_ascii.h"
#include "ppu.h" // LCD_WIDTH / LCD_HEIGHT

#define HALF_BLOCK "▀" // ▀ upper half block

static char tmp_path[64];

void setUp(void) {
    suppress_logs();

    strcpy(tmp_path, "/tmp/rascii_XXXXXX");
    int fd = mkstemp(tmp_path);
    if (fd != -1) close(fd);
}

void tearDown(void) {
    restore_logs();
    unlink(tmp_path);
}

// Read the whole temp file into a NUL-terminated heap buffer (output is text +
// escape codes, no embedded NULs). Caller frees. Returns byte count via *len.
static char *read_tmp(long *len) {
    FILE *f = fopen(tmp_path, "rb");
    TEST_ASSERT_NOT_NULL(f);
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(n + 1);
    size_t got = fread(buf, 1, n, f);
    buf[got] = '\0';
    fclose(f);
    if (len) *len = (long)got;
    return buf;
}

static int count_occurrences(const char *hay, const char *needle) {
    int count = 0;
    for (const char *p = strstr(hay, needle); p; p = strstr(p + 1, needle)) {
        count++;
    }
    return count;
}

// Point the process stdout at the temp file so writes the renderer sends to its
// stdout fallback are captured instead of polluting the test output. Returns the
// saved fd to hand back to restore_stdout().
static int redirect_stdout_to_tmp(void) {
    fflush(stdout);
    int saved = dup(fileno(stdout));
    int fd = open(tmp_path, O_WRONLY | O_TRUNC);
    dup2(fd, fileno(stdout));
    close(fd);
    return saved;
}

static void restore_stdout(int saved) {
    fflush(stdout);
    dup2(saved, fileno(stdout));
    close(saved);
    setvbuf(stdout, NULL, _IOLBF, 0); // init() enlarged stdout's buffer; restore
}

// ---- constructor ----

void test_renderer_ascii_returns_populated_vtable(void) {
    renderer_t r = renderer_ascii(NULL);

    TEST_ASSERT_NOT_NULL(r.init);
    TEST_ASSERT_NOT_NULL(r.render);
    TEST_ASSERT_NOT_NULL(r.deinit);
    TEST_ASSERT_NOT_NULL(r.ctx);
}

// ---- init ----

void test_renderer_ascii_init_opens_path_and_writes_setup(void) {
    renderer_t r = renderer_ascii(tmp_path);

    TEST_ASSERT_EQUAL_INT(0, r.init(&r));

    char *out = read_tmp(NULL);
    TEST_ASSERT_NOT_NULL(strstr(out, "\x1b[?25l")); // hide cursor
    TEST_ASSERT_NOT_NULL(strstr(out, "\x1b[2J"));    // clear screen
    free(out);

    r.deinit(&r);
}

void test_renderer_ascii_init_falls_back_to_stdout_on_bad_path(void) {
    int saved = redirect_stdout_to_tmp();

    renderer_t r = renderer_ascii("/no/such/dir/nope.tty");
    int rc = r.init(&r);

    restore_stdout(saved);

    TEST_ASSERT_EQUAL_INT(0, rc);
    char *out = read_tmp(NULL);
    TEST_ASSERT_NOT_NULL(strstr(out, "\x1b[?25l")); // setup went to stdout fallback
    free(out);
}

void test_renderer_ascii_init_null_path_uses_stdout(void) {
    int saved = redirect_stdout_to_tmp();

    renderer_t r = renderer_ascii(NULL); // no tty -> default to stdout
    int rc = r.init(&r);
    r.deinit(&r); // deinit must not fclose(stdout); done while redirected

    restore_stdout(saved);

    TEST_ASSERT_EQUAL_INT(0, rc);
    char *out = read_tmp(NULL);
    TEST_ASSERT_NOT_NULL(strstr(out, "\x1b[?25l"));
    free(out);
}

// ---- render ----

void test_renderer_ascii_render_emits_half_block_per_cell(void) {
    static uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT];
    memset(framebuffer, 0, sizeof(framebuffer));

    renderer_t r = renderer_ascii(tmp_path);
    r.init(&r);
    r.render(&r, framebuffer);

    char *out = read_tmp(NULL);
    // One glyph per character cell: LCD_WIDTH columns x LCD_HEIGHT/2 rows.
    TEST_ASSERT_EQUAL_INT(LCD_WIDTH * (LCD_HEIGHT / 2), count_occurrences(out, HALF_BLOCK));
    free(out);

    r.deinit(&r);
}

void test_renderer_ascii_render_uniform_frame_sends_color_once(void) {
    static uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT];
    memset(framebuffer, 0, sizeof(framebuffer)); // all shade 0 -> white

    renderer_t r = renderer_ascii(tmp_path);
    r.init(&r);
    r.render(&r, framebuffer);

    char *out = read_tmp(NULL);
    // Run-length optimization: the fg colour escape is emitted once for the
    // whole uniform frame, and shade 0 maps to white (255,255,255).
    TEST_ASSERT_EQUAL_INT(1, count_occurrences(out, "\x1b[38;2;255;255;255m"));
    free(out);

    r.deinit(&r);
}

void test_renderer_ascii_render_maps_dark_shade(void) {
    static uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT];
    memset(framebuffer, 0, sizeof(framebuffer));
    // Row 1 (the bottom pixel of cell row 0) is the darkest shade.
    memset(&framebuffer[LCD_WIDTH], 3, LCD_WIDTH);

    renderer_t r = renderer_ascii(tmp_path);
    r.init(&r);
    r.render(&r, framebuffer);

    char *out = read_tmp(NULL);
    // Shade 3 -> black background escape.
    TEST_ASSERT_NOT_NULL(strstr(out, "\x1b[48;2;0;0;0m"));
    free(out);

    r.deinit(&r);
}

void test_renderer_ascii_render_after_deinit_is_noop(void) {
    static uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT];
    memset(framebuffer, 0, sizeof(framebuffer));

    renderer_t r = renderer_ascii(tmp_path);
    r.init(&r);
    r.deinit(&r); // closes the stream and clears ctx->out

    long before;
    free(read_tmp(&before));

    r.render(&r, framebuffer); // must be ignored, stream is gone

    long after;
    free(read_tmp(&after));
    TEST_ASSERT_EQUAL_INT(before, after);
}

// ---- deinit ----

void test_renderer_ascii_deinit_restores_cursor(void) {
    renderer_t r = renderer_ascii(tmp_path);
    r.init(&r);

    r.deinit(&r);

    char *out = read_tmp(NULL);
    TEST_ASSERT_NOT_NULL(strstr(out, "\x1b[?25h")); // show cursor again
    free(out);
}

void test_renderer_ascii_deinit_twice_is_safe(void) {
    renderer_t r = renderer_ascii(tmp_path);
    r.init(&r);
    r.deinit(&r); // clears ctx->out

    long before;
    free(read_tmp(&before));

    r.deinit(&r); // second deinit must be an ignored no-op

    long after;
    free(read_tmp(&after));
    TEST_ASSERT_EQUAL_INT(before, after);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_renderer_ascii_returns_populated_vtable);

    RUN_TEST(test_renderer_ascii_init_opens_path_and_writes_setup);
    RUN_TEST(test_renderer_ascii_init_falls_back_to_stdout_on_bad_path);
    RUN_TEST(test_renderer_ascii_init_null_path_uses_stdout);

    RUN_TEST(test_renderer_ascii_render_emits_half_block_per_cell);
    RUN_TEST(test_renderer_ascii_render_uniform_frame_sends_color_once);
    RUN_TEST(test_renderer_ascii_render_maps_dark_shade);
    RUN_TEST(test_renderer_ascii_render_after_deinit_is_noop);

    RUN_TEST(test_renderer_ascii_deinit_restores_cursor);
    RUN_TEST(test_renderer_ascii_deinit_twice_is_safe);

    return UNITY_END();
}
