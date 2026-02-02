// INTEGRATION-TEST
// EXPECT: 1

// Test volatile 16-bit SIGNED less-than comparison
// Note: Comparing against non-zero constant avoids the lshr optimization bug

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile int val;

    val = -100;
    if (val < 50) result++;   // -100 < 50 is true (signed)

    val = 100;
    if (val < 50) result++;   // 100 < 50 is false

    return result;  // Should be 1
}
