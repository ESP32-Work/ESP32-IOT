#include <unity.h>
#include "add.h"


void test_addition() {
    TEST_ASSERT_EQUAL(5, add(2, 3));
    TEST_ASSERT_EQUAL(-1, add(-2, 1));
    TEST_ASSERT_EQUAL(0, add(0, 0));
}

void setUp() {}

void tearDown() {}

int main() {
    UNITY_BEGIN();  // Initialize Unity

    RUN_TEST(test_addition);  // Run the test

    UNITY_END();  // End Unity

    return 0;
}
