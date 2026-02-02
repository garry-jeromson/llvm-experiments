// INTEGRATION-TEST
// EXPECT: 8

// Test volatile 8-bit UNSIGNED comparisons - ALL operators
// Complements volatile_u8_compare.c which only tests <

static volatile unsigned char val;

unsigned int test_main(void) {
    unsigned int result = 0;

    // Test unsigned less-than (<) - already covered but include for completeness
    val = 3;
    if (val < 5) result++;     // 3 < 5 is true, result = 1

    val = 5;
    if (val < 5) result++;     // 5 < 5 is false, no increment

    // Test unsigned greater-than (>)
    val = 10;
    if (val > 5) result++;     // 10 > 5 is true, result = 2

    val = 5;
    if (val > 5) result++;     // 5 > 5 is false, no increment

    // Test unsigned less-or-equal (<=)
    val = 5;
    if (val <= 5) result++;    // 5 <= 5 is true, result = 3

    val = 3;
    if (val <= 5) result++;    // 3 <= 5 is true, result = 4

    val = 7;
    if (val <= 5) result++;    // 7 <= 5 is false, no increment

    // Test unsigned greater-or-equal (>=)
    val = 5;
    if (val >= 5) result++;    // 5 >= 5 is true, result = 5

    val = 7;
    if (val >= 5) result++;    // 7 >= 5 is true, result = 6

    val = 3;
    if (val >= 5) result++;    // 3 >= 5 is false, no increment

    // Test equality (==)
    val = 42;
    if (val == 42) result++;   // 42 == 42 is true, result = 7

    val = 41;
    if (val == 42) result++;   // 41 == 42 is false, no increment

    // Test inequality (!=)
    val = 41;
    if (val != 42) result++;   // 41 != 42 is true, result = 8

    val = 42;
    if (val != 42) result++;   // 42 != 42 is false, no increment

    return result;  // Should be 8
}
