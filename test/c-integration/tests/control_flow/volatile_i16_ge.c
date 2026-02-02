// INTEGRATION-TEST
// EXPECT: 2

// Test volatile 16-bit SIGNED greater-or-equal comparison
// Note: Comparing against non-zero constant avoids the lshr/XOR optimization bug

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile int val;

    val = 10;
    if (val >= 10) result++;    // 10 >= 10 is true

    val = 15;
    if (val >= 10) result++;    // 15 >= 10 is true

    val = 5;
    if (val >= 10) result++;    // 5 >= 10 is false

    return result;  // Should be 2
}
