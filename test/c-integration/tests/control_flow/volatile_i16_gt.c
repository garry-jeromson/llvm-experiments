// INTEGRATION-TEST
// EXPECT: 2

// Test volatile 16-bit SIGNED greater-than comparison

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile int val;

    val = 100;
    if (val > 0) result++;    // 100 > 0 is true

    val = -100;
    if (val > 0) result++;    // -100 > 0 is false

    val = 32767;
    if (val > 32766) result++; // 32767 > 32766 is true

    return result;  // Should be 2
}
