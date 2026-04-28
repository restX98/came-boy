#include "unity.h"

void setUp(void) {
    // This function is called before each test case
}

void tearDown(void) {
    // This function is called after each test case
}

void test_dummy(void) {
    // This is a dummy test case to ensure the test framework is working
    TEST_ASSERT_TRUE(1);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_dummy);

    return UNITY_END();
}
