// INTEGRATION-TEST
// EXPECT: 100
// Regression test: RELOAD_GPR16 between CMP and Select must preserve flags.
// The bug was that LDA in RELOAD clobbers N/Z flags, but Select reads them.
// Fix: PHP/PLP around RELOAD with adjusted stack offset.

static volatile int va, vb, vc, vd;

// Test function with 4 args that need to be combined
__attribute__((noinline))
int four_args(int a, int b, int c, int d) {
    return a + b - c + d;
}

// Test with complex expression - forces spill/reload patterns
__attribute__((noinline))
int complex_expr(int a, int b) {
    int t1 = a * 2;
    int t2 = b * 3;
    int t3 = t1 + t2;
    int t4 = t3 & 0xFF;
    return t4;
}

int test_main(void) {
    va = 10;
    vb = 20;
    vc = 5;
    vd = 15;

    // four_args(10, 20, 5, 15) = 10 + 20 - 5 + 15 = 40
    int r1 = four_args(va, vb, vc, vd);
    if (r1 != 40) return r1;

    // complex_expr(10, 20) = ((10*2) + (20*3)) & 0xFF = (20 + 60) & 0xFF = 80
    // The select pattern here triggers RELOAD between CMP and Select
    int r2 = complex_expr(va, vb);
    if (r2 != 80) return r2 + 100;

    return 100;
}
