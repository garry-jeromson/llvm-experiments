// INTEGRATION-TEST
// EXPECT: 6

// Test 8-bit SIGNED boundary values in comparisons
// Tests INT8_MIN (-128) and INT8_MAX (127) edge cases

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile signed char val;

    // INT8_MIN tests
    val = -128;
    if (val < 0) result++;     // -128 < 0 is true, result = 1
    if (val < -127) result++;  // -128 < -127 is true, result = 2
    if (val <= -128) result++; // -128 <= -128 is true, result = 3

    // INT8_MAX tests
    val = 127;
    if (val > 0) result++;     // 127 > 0 is true, result = 4
    if (val > 126) result++;   // 127 > 126 is true, result = 5
    if (val >= 127) result++;  // 127 >= 127 is true, result = 6

    // Edge case: compare INT8_MIN and INT8_MAX
    val = -128;
    if (val > 127) result++;   // -128 > 127 is false (signed), no increment

    val = 127;
    if (val < -128) result++;  // 127 < -128 is false (signed), no increment

    return result;  // Should be 6
}
