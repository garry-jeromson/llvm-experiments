// INTEGRATION-TEST
// EXPECT: 85

// Interleaved operations forcing register swaps
int test_main(void) {
    int a = 5, b = 10, c = 15, d = 20;
    int r1 = a + b;    // 15
    int r2 = c + d;    // 35
    int r3 = a * 2;    // 10
    int r4 = b + c;    // 25
    int r5 = r1 + r3;  // 25
    int r6 = r2 - r4;  // 10
    return r5 + r6 + a + b + c + d;  // 25 + 10 + 5 + 10 + 15 + 20 = 85
}
