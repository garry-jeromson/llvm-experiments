// INTEGRATION-TEST
// EXPECT: 200

// Large expression tree requiring many temporaries
int test_main(void) {
    int a = 2, b = 3, c = 4, d = 5, e = 6, f = 7, g = 8, h = 9;

    // Deep expression tree
    int result = ((a + b) * (c + d)) +
                 ((e + f) * (g + h)) -
                 ((a * b) + (c * d)) +
                 ((e * f) - (g * h));
    // (5 * 9) + (13 * 17) - (6 + 20) + (42 - 72)
    // 45 + 221 - 26 + (-30)
    // 45 + 221 - 26 - 30 = 210
    return result - 10;  // 200
}
