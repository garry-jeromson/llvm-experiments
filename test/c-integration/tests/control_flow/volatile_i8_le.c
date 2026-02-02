// INTEGRATION-TEST
// EXPECT: 3

// Test volatile 8-bit SIGNED less-or-equal comparison

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile signed char val;

    val = 0;
    if (val <= 0) result++;    // 0 <= 0 is true

    val = -1;
    if (val <= 0) result++;    // -1 <= 0 is true

    val = 1;
    if (val <= 0) result++;    // 1 <= 0 is false

    val = -128;
    if (val <= -128) result++; // -128 <= -128 is true

    return result;  // Should be 3
}
