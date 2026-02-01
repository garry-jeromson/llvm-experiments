// INTEGRATION-TEST
// EXPECT: 1360

// Sixteen simultaneous live variables (matches RS0-RS15 imaginary regs)
int test_main(void) {
    int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60, g = 70, h = 80;
    int i = 90, j = 100, k = 110, l = 120, m = 130, n = 140, o = 150, p = 160;
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}
