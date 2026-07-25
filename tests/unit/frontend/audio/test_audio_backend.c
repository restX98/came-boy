#include "unity.h"
#include "log_helpers.h"

#include "frontend/audio/audio_backend.h"

// ---- Fake backend ----

static size_t init_calls;
static audio_backend_t *init_self;
static int init_return;

static int fake_init(audio_backend_t *self) {
    init_calls++;
    init_self = self;
    return init_return;
}

static size_t queue_sample_calls;
static audio_backend_t *queue_sample_self;
static int16_t queue_sample_left;
static int16_t queue_sample_right;

static void fake_queue_sample(audio_backend_t *self, int16_t left, int16_t right) {
    queue_sample_calls++;
    queue_sample_self = self;
    queue_sample_left = left;
    queue_sample_right = right;
}

static size_t deinit_calls;
static audio_backend_t *deinit_self;

static void fake_deinit(audio_backend_t *self) {
    deinit_calls++;
    deinit_self = self;
}

static audio_backend_t make_full_backend(void) {
    return (audio_backend_t){
        .init = fake_init,
        .queue_sample = fake_queue_sample,
        .deinit = fake_deinit,
        .ctx = NULL,
    };
}

void setUp(void) {
    suppress_logs();

    init_calls = 0;
    init_self = NULL;
    init_return = 0;
    queue_sample_calls = 0;
    queue_sample_self = NULL;
    queue_sample_left = 0;
    queue_sample_right = 0;
    deinit_calls = 0;
    deinit_self = NULL;
}

void tearDown(void) {
    restore_logs();
}

// ---- audio_backend_init ----

void test_audio_backend_init_null_backend_returns_0(void) {
    TEST_ASSERT_EQUAL_INT(0, audio_backend_init(NULL));
}

void test_audio_backend_init_null_hook_returns_0(void) {
    audio_backend_t b = { 0 }; // init hook is NULL

    TEST_ASSERT_EQUAL_INT(0, audio_backend_init(&b));
    TEST_ASSERT_EQUAL_size_t(0, init_calls);
}

void test_audio_backend_init_calls_hook_and_returns_value(void) {
    audio_backend_t b = make_full_backend();
    init_return = 42;

    int result = audio_backend_init(&b);

    TEST_ASSERT_EQUAL_INT(42, result);
    TEST_ASSERT_EQUAL_size_t(1, init_calls);
    TEST_ASSERT_EQUAL_PTR(&b, init_self);
}

// ---- audio_backend_queue_sample ----

void test_audio_backend_queue_sample_null_backend_is_noop(void) {
    audio_backend_queue_sample(NULL, 1, -1);
    TEST_ASSERT_EQUAL_size_t(0, queue_sample_calls);
}

void test_audio_backend_queue_sample_null_hook_is_noop(void) {
    audio_backend_t b = { 0 }; // queue_sample hook is NULL

    audio_backend_queue_sample(&b, 1, -1);

    TEST_ASSERT_EQUAL_size_t(0, queue_sample_calls);
}

void test_audio_backend_queue_sample_calls_hook_with_samples(void) {
    audio_backend_t b = make_full_backend();

    audio_backend_queue_sample(&b, 1234, -1234);

    TEST_ASSERT_EQUAL_size_t(1, queue_sample_calls);
    TEST_ASSERT_EQUAL_PTR(&b, queue_sample_self);
    TEST_ASSERT_EQUAL_INT16(1234, queue_sample_left);
    TEST_ASSERT_EQUAL_INT16(-1234, queue_sample_right);
}

// ---- audio_backend_deinit ----

void test_audio_backend_deinit_null_backend_is_noop(void) {
    audio_backend_deinit(NULL);
    TEST_ASSERT_EQUAL_size_t(0, deinit_calls);
}

void test_audio_backend_deinit_null_hook_is_noop(void) {
    audio_backend_t b = { 0 }; // deinit hook is NULL

    audio_backend_deinit(&b);

    TEST_ASSERT_EQUAL_size_t(0, deinit_calls);
}

void test_audio_backend_deinit_calls_hook(void) {
    audio_backend_t b = make_full_backend();

    audio_backend_deinit(&b);

    TEST_ASSERT_EQUAL_size_t(1, deinit_calls);
    TEST_ASSERT_EQUAL_PTR(&b, deinit_self);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_audio_backend_init_null_backend_returns_0);
    RUN_TEST(test_audio_backend_init_null_hook_returns_0);
    RUN_TEST(test_audio_backend_init_calls_hook_and_returns_value);

    RUN_TEST(test_audio_backend_queue_sample_null_backend_is_noop);
    RUN_TEST(test_audio_backend_queue_sample_null_hook_is_noop);
    RUN_TEST(test_audio_backend_queue_sample_calls_hook_with_samples);

    RUN_TEST(test_audio_backend_deinit_null_backend_is_noop);
    RUN_TEST(test_audio_backend_deinit_null_hook_is_noop);
    RUN_TEST(test_audio_backend_deinit_calls_hook);

    return UNITY_END();
}
