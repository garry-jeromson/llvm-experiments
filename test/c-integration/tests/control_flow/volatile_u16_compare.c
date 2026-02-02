// INTEGRATION-TEST
// EXPECT: 8

// Test volatile 16-bit UNSIGNED comparisons
// Covers all comparison operators with volatile unsigned int

static volatile unsigned int val;

unsigned int test_main(void) {
    unsigned int result = 0;

    // Test unsigned less-than (<)
    val = 10;
    if (val < 100) result++;   // 10 < 100 is true, result = 1

    val = 100;
    if (val < 10) result++;    // 100 < 10 is false, no increment

    // Test unsigned greater-than (>)
    val = 100;
    if (val > 10) result++;    // 100 > 10 is true, result = 2

    val = 10;
    if (val > 100) result++;   // 10 > 100 is false, no increment

    // Test unsigned less-or-equal (<=)
    val = 10;
    if (val <= 10) result++;   // 10 <= 10 is true, result = 3

    val = 5;
    if (val <= 10) result++;   // 5 <= 10 is true, result = 4

    val = 15;
    if (val <= 10) result++;   // 15 <= 10 is false, no increment

    // Test unsigned greater-or-equal (>=)
    val = 10;
    if (val >= 10) result++;   // 10 >= 10 is true, result = 5

    val = 15;
    if (val >= 10) result++;   // 15 >= 10 is true, result = 6

    val = 5;
    if (val >= 10) result++;   // 5 >= 10 is false, no increment

    // Test equality (==)
    val = 1000;
    if (val == 1000) result++; // 1000 == 1000 is true, result = 7

    val = 999;
    if (val == 1000) result++; // 999 == 1000 is false, no increment

    // Test inequality (!=)
    val = 999;
    if (val != 1000) result++; // 999 != 1000 is true, result = 8

    val = 1000;
    if (val != 1000) result++; // 1000 != 1000 is false, no increment

    return result;  // Should be 8
}
