#include "unity.h"
#include "log_helpers.h"

#include "mem.h"

static mem_t memory;

void setUp(void) {
    suppress_logs();
    memory = (mem_t){ 0 };
}

void tearDown(void) {
    mem_free(&memory);
    restore_logs();
}

// ---- mem_init ----

void test_mem_init_sets_name(void) {
    mem_init(&memory, 1024, "test");
    TEST_ASSERT_EQUAL_STRING("test", memory.name);
}

void test_mem_init_allocates_memory(void) {
    int result = mem_init(&memory, 1024, "test");
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_NOT_NULL(memory.mem);
    TEST_ASSERT_EQUAL_size_t(1024, memory.size);
}

// ---- mem_free ----

void test_mem_free_sets_mem_to_null(void) {
    mem_init(&memory, 0, "test");
    mem_free(&memory);
    TEST_ASSERT_NULL(memory.mem);
    TEST_ASSERT_EQUAL_size_t(0, memory.size);
}

void test_mem_free_does_not_crash_on_null_pointer(void) {
    mem_free(NULL); // should not crash
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mem_init_sets_name);
    RUN_TEST(test_mem_init_allocates_memory);
    RUN_TEST(test_mem_free_sets_mem_to_null);
    RUN_TEST(test_mem_free_does_not_crash_on_null_pointer);

    return UNITY_END();
}
