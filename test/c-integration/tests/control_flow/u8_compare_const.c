// INTEGRATION-TEST
// EXPECT: 3

// Test 8-bit unsigned comparison against small constant
// This pattern failed in the SNES demo: if (menu_sel < 4) menu_sel++

unsigned char test_main(void) {
    volatile unsigned char x = 0;
    unsigned char result = 0;

    // Should increment when x < 4
    x = 0;
    if (x < 4) result++;  // Should increment (0 < 4)

    x = 2;
    if (x < 4) result++;  // Should increment (2 < 4)

    x = 3;
    if (x < 4) result++;  // Should increment (3 < 4)

    x = 4;
    if (x < 4) result++;  // Should NOT increment (4 is not < 4)

    x = 5;
    if (x < 4) result++;  // Should NOT increment (5 is not < 4)

    return result;  // Expect 3
}
