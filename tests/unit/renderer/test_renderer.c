#include "unity.h"
#include "log_helpers.h"

#include "renderer/renderer.h"

// ---- Fake backend ----

static size_t init_calls;
static renderer_t *init_self;
static int init_return;

static int fake_init(renderer_t *self) {
    init_calls++;
    init_self = self;
    return init_return;
}

static size_t render_calls;
static renderer_t *render_self;
static const uint8_t *render_fb;

static void fake_render(renderer_t *self, const uint8_t *framebuffer) {
    render_calls++;
    render_self = self;
    render_fb = framebuffer;
}

static size_t deinit_calls;
static renderer_t *deinit_self;

static void fake_deinit(renderer_t *self) {
    deinit_calls++;
    deinit_self = self;
}

static renderer_t make_full_renderer(void) {
    return (renderer_t){
        .init = fake_init,
        .render = fake_render,
        .deinit = fake_deinit,
        .ctx = NULL,
    };
}

void setUp(void) {
    suppress_logs();

    init_calls = 0;
    init_self = NULL;
    init_return = 0;
    render_calls = 0;
    render_self = NULL;
    render_fb = NULL;
    deinit_calls = 0;
    deinit_self = NULL;
}

void tearDown(void) {
    restore_logs();
}

// ---- renderer_init ----

void test_renderer_init_null_renderer_returns_0(void) {
    TEST_ASSERT_EQUAL_INT(0, renderer_init(NULL));
}

void test_renderer_init_null_hook_returns_0(void) {
    renderer_t r = { 0 }; // init hook is NULL

    TEST_ASSERT_EQUAL_INT(0, renderer_init(&r));
    TEST_ASSERT_EQUAL_size_t(0, init_calls);
}

void test_renderer_init_calls_hook_and_returns_value(void) {
    renderer_t r = make_full_renderer();
    init_return = 42;

    int result = renderer_init(&r);

    TEST_ASSERT_EQUAL_INT(42, result);
    TEST_ASSERT_EQUAL_size_t(1, init_calls);
    TEST_ASSERT_EQUAL_PTR(&r, init_self);
}

// ---- renderer_render ----

void test_renderer_render_null_renderer_is_noop(void) {
    renderer_render(NULL, NULL);
    TEST_ASSERT_EQUAL_size_t(0, render_calls);
}

void test_renderer_render_null_hook_is_noop(void) {
    renderer_t r = { 0 }; // render hook is NULL

    renderer_render(&r, NULL);

    TEST_ASSERT_EQUAL_size_t(0, render_calls);
}

void test_renderer_render_calls_hook_with_framebuffer(void) {
    renderer_t r = make_full_renderer();
    uint8_t framebuffer[4] = { 0 };

    renderer_render(&r, framebuffer);

    TEST_ASSERT_EQUAL_size_t(1, render_calls);
    TEST_ASSERT_EQUAL_PTR(&r, render_self);
    TEST_ASSERT_EQUAL_PTR(framebuffer, render_fb);
}

// ---- renderer_deinit ----

void test_renderer_deinit_null_renderer_is_noop(void) {
    renderer_deinit(NULL);
    TEST_ASSERT_EQUAL_size_t(0, deinit_calls);
}

void test_renderer_deinit_null_hook_is_noop(void) {
    renderer_t r = { 0 }; // deinit hook is NULL

    renderer_deinit(&r);

    TEST_ASSERT_EQUAL_size_t(0, deinit_calls);
}

void test_renderer_deinit_calls_hook(void) {
    renderer_t r = make_full_renderer();

    renderer_deinit(&r);

    TEST_ASSERT_EQUAL_size_t(1, deinit_calls);
    TEST_ASSERT_EQUAL_PTR(&r, deinit_self);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_renderer_init_null_renderer_returns_0);
    RUN_TEST(test_renderer_init_null_hook_returns_0);
    RUN_TEST(test_renderer_init_calls_hook_and_returns_value);

    RUN_TEST(test_renderer_render_null_renderer_is_noop);
    RUN_TEST(test_renderer_render_null_hook_is_noop);
    RUN_TEST(test_renderer_render_calls_hook_with_framebuffer);

    RUN_TEST(test_renderer_deinit_null_renderer_is_noop);
    RUN_TEST(test_renderer_deinit_null_hook_is_noop);
    RUN_TEST(test_renderer_deinit_calls_hook);

    return UNITY_END();
}
