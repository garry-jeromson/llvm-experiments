// INTEGRATION-TEST
// EXPECT: 2

// Test volatile 8-bit SIGNED greater-than comparison

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile signed char val;

    val = 5;
    if (val > 0) result++;    // 5 > 0 is true

    val = -5;
    if (val > 0) result++;    // -5 > 0 is false

    val = 127;
    if (val > 126) result++;  // 127 > 126 is true

    return result;  // Should be 2
}
