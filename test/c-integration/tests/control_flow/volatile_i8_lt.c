// INTEGRATION-TEST
// EXPECT: 1
// SKIP: Signed less-than comparison causes register allocation failure with volatile variables

// Test volatile 8-bit SIGNED less-than comparison

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile signed char val;

    val = -5;
    if (val < 0) result++;    // -5 < 0 is true

    val = 5;
    if (val < 0) result++;    // 5 < 0 is false

    return result;  // Should be 1
}
