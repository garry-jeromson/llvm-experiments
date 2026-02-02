// INTEGRATION-TEST
// EXPECT: 2
// SKIP: Signed less-or-equal comparison causes register allocation failure with volatile variables

// Test volatile 16-bit SIGNED less-or-equal comparison

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile int val;

    val = 0;
    if (val <= 0) result++;     // 0 <= 0 is true

    val = -1;
    if (val <= 0) result++;     // -1 <= 0 is true

    val = 1;
    if (val <= 0) result++;     // 1 <= 0 is false

    return result;  // Should be 2
}
