// INTEGRATION-TEST
// EXPECT: 6

// Test 8-bit UNSIGNED boundary values in comparisons
// Tests 0 and 255 (UINT8_MAX) edge cases

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile unsigned char val;

    // Zero boundary tests
    val = 0;
    if (val == 0) result++;    // 0 == 0 is true, result = 1
    if (val <= 0) result++;    // 0 <= 0 is true, result = 2
    if (val < 1) result++;     // 0 < 1 is true, result = 3

    val = 0;
    if (val > 0) result++;     // 0 > 0 is false, no increment

    // UINT8_MAX tests
    val = 255;
    if (val == 255) result++;  // 255 == 255 is true, result = 4
    if (val >= 255) result++;  // 255 >= 255 is true, result = 5
    if (val > 254) result++;   // 255 > 254 is true, result = 6

    val = 255;
    if (val < 255) result++;   // 255 < 255 is false, no increment

    // Compare 0 and 255 (unsigned)
    val = 0;
    if (val > 255) result++;   // 0 > 255 is false (unsigned), no increment

    val = 255;
    if (val < 0) result++;     // 255 < 0 is false (unsigned), no increment

    return result;  // Should be 6
}
