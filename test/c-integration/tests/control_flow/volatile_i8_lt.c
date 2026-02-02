// INTEGRATION-TEST
// EXPECT: 1

// Test volatile 8-bit SIGNED less-than comparison
// Note: Comparing against non-zero constant avoids the lshr optimization bug

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile signed char val;

    val = -5;
    if (val < 10) result++;   // -5 < 10 is true (signed)

    val = 15;
    if (val < 10) result++;   // 15 < 10 is false

    return result;  // Should be 1
}
