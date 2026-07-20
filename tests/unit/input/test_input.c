#include "unity.h"
#include "log_helpers.h"

#include "input/input.h"

// ---- Fake backend ----

static size_t init_calls;
static input_t *init_self;
static int init_return;

static int fake_init(input_t *self) {
    init_calls++;
    init_self = self;
    return init_return;
}

static size_t poll_calls;
static input_t *poll_self;
static joypad_reg_t *poll_jp;
static interrupt_regs_t *poll_interrupts;

static void fake_poll(input_t *self, joypad_reg_t *jp, interrupt_regs_t *interrupts) {
    poll_calls++;
    poll_self = self;
    poll_jp = jp;
    poll_interrupts = interrupts;
}

static size_t deinit_calls;
static input_t *deinit_self;

static void fake_deinit(input_t *self) {
    deinit_calls++;
    deinit_self = self;
}

static input_t make_full_input(void) {
    return (input_t){
        .init = fake_init,
        .poll = fake_poll,
        .deinit = fake_deinit,
        .ctx = NULL,
    };
}

void setUp(void) {
    suppress_logs();

    init_calls = 0;
    init_self = NULL;
    init_return = 0;
    poll_calls = 0;
    poll_self = NULL;
    poll_jp = NULL;
    poll_interrupts = NULL;
    deinit_calls = 0;
    deinit_self = NULL;
}

void tearDown(void) {
    restore_logs();
}

// ---- input_init ----

void test_input_init_null_input_returns_0(void) {
    TEST_ASSERT_EQUAL_INT(0, input_init(NULL));
}

void test_input_init_null_hook_returns_0(void) {
    input_t in = { 0 }; // init hook is NULL

    TEST_ASSERT_EQUAL_INT(0, input_init(&in));
    TEST_ASSERT_EQUAL_size_t(0, init_calls);
}

void test_input_init_calls_hook_and_returns_value(void) {
    input_t in = make_full_input();
    init_return = 42;

    int result = input_init(&in);

    TEST_ASSERT_EQUAL_INT(42, result);
    TEST_ASSERT_EQUAL_size_t(1, init_calls);
    TEST_ASSERT_EQUAL_PTR(&in, init_self);
}

// ---- input_poll ----

void test_input_poll_null_input_is_noop(void) {
    joypad_reg_t jp = { 0 };
    interrupt_regs_t interrupts = { 0 };

    input_poll(NULL, &jp, &interrupts);

    TEST_ASSERT_EQUAL_size_t(0, poll_calls);
}

void test_input_poll_null_hook_is_noop(void) {
    input_t in = { 0 }; // poll hook is NULL
    joypad_reg_t jp = { 0 };
    interrupt_regs_t interrupts = { 0 };

    input_poll(&in, &jp, &interrupts);

    TEST_ASSERT_EQUAL_size_t(0, poll_calls);
}

void test_input_poll_calls_hook_with_args(void) {
    input_t in = make_full_input();
    joypad_reg_t jp = { 0 };
    interrupt_regs_t interrupts = { 0 };

    input_poll(&in, &jp, &interrupts);

    TEST_ASSERT_EQUAL_size_t(1, poll_calls);
    TEST_ASSERT_EQUAL_PTR(&in, poll_self);
    TEST_ASSERT_EQUAL_PTR(&jp, poll_jp);
    TEST_ASSERT_EQUAL_PTR(&interrupts, poll_interrupts);
}

// ---- input_deinit ----

void test_input_deinit_null_input_is_noop(void) {
    input_deinit(NULL);

    TEST_ASSERT_EQUAL_size_t(0, deinit_calls);
}

void test_input_deinit_null_hook_is_noop(void) {
    input_t in = { 0 }; // deinit hook is NULL

    input_deinit(&in);

    TEST_ASSERT_EQUAL_size_t(0, deinit_calls);
}

void test_input_deinit_calls_hook(void) {
    input_t in = make_full_input();

    input_deinit(&in);

    TEST_ASSERT_EQUAL_size_t(1, deinit_calls);
    TEST_ASSERT_EQUAL_PTR(&in, deinit_self);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_input_init_null_input_returns_0);
    RUN_TEST(test_input_init_null_hook_returns_0);
    RUN_TEST(test_input_init_calls_hook_and_returns_value);

    RUN_TEST(test_input_poll_null_input_is_noop);
    RUN_TEST(test_input_poll_null_hook_is_noop);
    RUN_TEST(test_input_poll_calls_hook_with_args);

    RUN_TEST(test_input_deinit_null_input_is_noop);
    RUN_TEST(test_input_deinit_null_hook_is_noop);
    RUN_TEST(test_input_deinit_calls_hook);

    return UNITY_END();
}
