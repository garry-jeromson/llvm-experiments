// INTEGRATION-TEST
// EXPECT: 1
// SKIP: Signed less-than comparison causes register allocation failure with volatile variables

// Test volatile 16-bit SIGNED less-than comparison

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile int val;

    val = -100;
    if (val < 0) result++;    // -100 < 0 is true

    val = 100;
    if (val < 0) result++;    // 100 < 0 is false

    return result;  // Should be 1
}
