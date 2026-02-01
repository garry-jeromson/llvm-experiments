// INTEGRATION-TEST
// EXPECT: 780

// Twelve simultaneous live variables
int test_main(void) {
    int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60;
    int g = 70, h = 80, i = 90, j = 100, k = 110, l = 120;
    return a + b + c + d + e + f + g + h + i + j + k + l;  // 780
}
