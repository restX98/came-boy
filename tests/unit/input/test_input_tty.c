#define _GNU_SOURCE // posix_openpt/grantpt/unlockpt/ptsname

#include "unity.h"
#include "log_helpers.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "input/input_tty.h"

// ---- Stubs / spies ---------------------------------------------------------
// input_tty only depends on the joypad frontend, so stub it out and record the
// keys the poll loop presses/releases.

#define MAX_EVENTS 64

static joypad_key_t pressed[MAX_EVENTS];
static size_t press_calls;

void joypad_press(joypad_reg_t *jp, interrupt_regs_t *interrupts, joypad_key_t key) {
    (void)jp;
    (void)interrupts;
    if (press_calls < MAX_EVENTS) pressed[press_calls] = key;
    press_calls++;
}

static joypad_key_t released[MAX_EVENTS];
static size_t release_calls;

void joypad_release(joypad_reg_t *jp, joypad_key_t key) {
    (void)jp;
    if (release_calls < MAX_EVENTS) released[release_calls] = key;
    release_calls++;
}

static void reset_spies(void) {
    press_calls = 0;
    release_calls = 0;
}

static bool pressed_contains(joypad_key_t key) {
    size_t n = press_calls < MAX_EVENTS ? press_calls : MAX_EVENTS;
    for (size_t i = 0; i < n; i++)
        if (pressed[i] == key) return true;
    return false;
}

static bool released_contains(joypad_key_t key) {
    size_t n = release_calls < MAX_EVENTS ? release_calls : MAX_EVENTS;
    for (size_t i = 0; i < n; i++)
        if (released[i] == key) return true;
    return false;
}

// ---- pty harness -----------------------------------------------------------

static int pty_master;
static char pty_slave_path[128];

static void open_pty(void) {
    pty_master = posix_openpt(O_RDWR | O_NOCTTY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, pty_master);
    TEST_ASSERT_EQUAL_INT(0, grantpt(pty_master));
    TEST_ASSERT_EQUAL_INT(0, unlockpt(pty_master));

    const char *p = ptsname(pty_master);
    TEST_ASSERT_NOT_NULL(p);
    strncpy(pty_slave_path, p, sizeof(pty_slave_path) - 1);
    pty_slave_path[sizeof(pty_slave_path) - 1] = '\0';
}

static void msleep(long ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

void setUp(void) {
    suppress_logs();

    reset_spies();
    pty_master = -1;
    pty_slave_path[0] = '\0';
}

void tearDown(void) {
    restore_logs();
    if (pty_master >= 0) close(pty_master);
}

// ---- input_tty (construction) ----

void test_input_tty_returns_wired_backend(void) {
    input_t in = input_tty("/dev/null");

    TEST_ASSERT_NOT_NULL(in.init);
    TEST_ASSERT_NOT_NULL(in.poll);
    TEST_ASSERT_NOT_NULL(in.deinit);
    TEST_ASSERT_NOT_NULL(in.ctx);
}

void test_input_tty_null_path_still_wires_backend(void) {
    input_t in = input_tty(NULL);

    TEST_ASSERT_NOT_NULL(in.init);
    TEST_ASSERT_NOT_NULL(in.poll);
    TEST_ASSERT_NOT_NULL(in.deinit);
    TEST_ASSERT_NOT_NULL(in.ctx);
}

// ---- init ----

void test_input_tty_init_sets_raw_mode(void) {
    open_pty();
    int sfd = open(pty_slave_path, O_RDONLY | O_NOCTTY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, sfd);

    input_t in = input_tty(pty_slave_path);
    in.init(&in);

    struct termios after;
    TEST_ASSERT_EQUAL_INT(0, tcgetattr(sfd, &after));
    TEST_ASSERT_EQUAL_INT(0, after.c_lflag & (ICANON | ECHO)); // canonical + echo off
    TEST_ASSERT_EQUAL_UINT8(0, after.c_cc[VMIN]);              // polling read
    TEST_ASSERT_EQUAL_UINT8(0, after.c_cc[VTIME]);

    in.deinit(&in);
    close(sfd);
}

// ---- poll (key decoding) ----

void test_input_tty_poll_decodes_keys(void) {
    open_pty();
    input_t in = input_tty(pty_slave_path);
    in.init(&in);

    // Every mapped key, in order, plus an unmapped byte that must be ignored.
    const char *seq = "wsadlpvbz";
    TEST_ASSERT_EQUAL_INT((int)strlen(seq), (int)write(pty_master, seq, strlen(seq)));
    msleep(5); // let the line discipline hand the bytes to the slave

    joypad_reg_t jp = { 0 };
    interrupt_regs_t interrupts = { 0 };
    in.poll(&in, &jp, &interrupts);

    TEST_ASSERT_EQUAL_size_t(8, press_calls); // 'z' ignored
    TEST_ASSERT_EQUAL(JOYPAD_UP, pressed[0]);
    TEST_ASSERT_EQUAL(JOYPAD_DOWN, pressed[1]);
    TEST_ASSERT_EQUAL(JOYPAD_LEFT, pressed[2]);
    TEST_ASSERT_EQUAL(JOYPAD_RIGHT, pressed[3]);
    TEST_ASSERT_EQUAL(JOYPAD_A, pressed[4]);
    TEST_ASSERT_EQUAL(JOYPAD_B, pressed[5]);
    TEST_ASSERT_EQUAL(JOYPAD_SELECT, pressed[6]);
    TEST_ASSERT_EQUAL(JOYPAD_START, pressed[7]);

    in.deinit(&in);
}

// ---- poll (auto-release) ----

void test_input_tty_poll_auto_releases_after_timeout(void) {
    open_pty();
    input_t in = input_tty(pty_slave_path);
    in.init(&in);

    joypad_reg_t jp = { 0 };
    interrupt_regs_t interrupts = { 0 };

    TEST_ASSERT_EQUAL_INT(1, (int)write(pty_master, "w", 1));
    msleep(5);
    in.poll(&in, &jp, &interrupts); // press UP
    TEST_ASSERT_TRUE(pressed_contains(JOYPAD_UP));

    // Still within RELEASE_MS: UP must NOT be released yet.
    reset_spies();
    in.poll(&in, &jp, &interrupts);
    TEST_ASSERT_FALSE(released_contains(JOYPAD_UP));

    // After the quiet window (RELEASE_MS is 120ms in the backend): UP released.
    reset_spies();
    msleep(200);
    in.poll(&in, &jp, &interrupts);
    TEST_ASSERT_TRUE(released_contains(JOYPAD_UP));

    in.deinit(&in);
}

// ---- deinit ----

void test_input_tty_deinit_restores_termios(void) {
    open_pty();
    int sfd = open(pty_slave_path, O_RDONLY | O_NOCTTY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, sfd);

    struct termios before;
    TEST_ASSERT_EQUAL_INT(0, tcgetattr(sfd, &before));

    input_t in = input_tty(pty_slave_path);
    in.init(&in);   // flips to raw
    in.deinit(&in); // must restore the saved state

    struct termios after;
    TEST_ASSERT_EQUAL_INT(0, tcgetattr(sfd, &after));
    TEST_ASSERT_EQUAL_UINT32(before.c_lflag, after.c_lflag);

    close(sfd);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_input_tty_returns_wired_backend);
    RUN_TEST(test_input_tty_null_path_still_wires_backend);

    RUN_TEST(test_input_tty_init_sets_raw_mode);
    RUN_TEST(test_input_tty_poll_decodes_keys);
    RUN_TEST(test_input_tty_poll_auto_releases_after_timeout);
    RUN_TEST(test_input_tty_deinit_restores_termios);

    return UNITY_END();
}
