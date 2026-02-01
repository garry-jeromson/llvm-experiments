// INTEGRATION-TEST
// EXPECT: 2100

// Twenty simultaneous live variables (exceeds imaginary regs, needs stack spills)
int test_main(void) {
    int a = 10, b = 20, c = 30, d = 40, e = 50;
    int f = 60, g = 70, h = 80, i = 90, j = 100;
    int k = 110, l = 120, m = 130, n = 140, o = 150;
    int p = 160, q = 170, r = 180, s = 190, t = 200;
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t;
}
