// INTEGRATION-TEST
// EXPECT: 100
// Test min/max/abs patterns that were previously unsupported (G_SMAX, G_SMIN,
// G_UMAX, G_UMIN, G_ABS). These are common C patterns that LLVM recognizes.

// Signed max via ternary
__attribute__((noinline))
int max_signed(int a, int b) {
    return (a > b) ? a : b;
}

// Signed min via ternary
__attribute__((noinline))
int min_signed(int a, int b) {
    return (a < b) ? a : b;
}

// Unsigned max via ternary
__attribute__((noinline))
unsigned max_unsigned(unsigned a, unsigned b) {
    return (a > b) ? a : b;
}

// Unsigned min via ternary
__attribute__((noinline))
unsigned min_unsigned(unsigned a, unsigned b) {
    return (a < b) ? a : b;
}

// Absolute value via ternary
__attribute__((noinline))
int my_abs(int x) {
    return (x < 0) ? -x : x;
}

// Conditional accumulation pattern (generates G_SMAX with 0 internally)
__attribute__((noinline))
int cond_accum(int a, int b, int c) {
    int result = 0;
    if (a > 0) result += a;
    if (b > 0) result += b;
    if (c > 0) result += c;
    return result;
}

int test_main(void) {
    // Test signed max
    // max_signed(5, 3) = 5
    int r1 = max_signed(5, 3);
    if (r1 != 5) return r1;

    // max_signed(-5, -3) = -3
    int r2 = max_signed(-5, -3);
    if (r2 != -3) return 10;

    // Test signed min
    // min_signed(5, 3) = 3
    int r3 = min_signed(5, 3);
    if (r3 != 3) return r3 + 20;

    // min_signed(-5, -3) = -5
    int r4 = min_signed(-5, -3);
    if (r4 != -5) return 30;

    // Test unsigned max
    // max_unsigned(5, 3) = 5
    unsigned r5 = max_unsigned(5, 3);
    if (r5 != 5) return r5 + 40;

    // Test unsigned min
    // min_unsigned(5, 3) = 3
    unsigned r6 = min_unsigned(5, 3);
    if (r6 != 3) return r6 + 50;

    // Test abs with negative
    // my_abs(-7) = 7
    int r7 = my_abs(-7);
    if (r7 != 7) return r7 + 60;

    // Test abs with positive
    // my_abs(7) = 7
    int r8 = my_abs(7);
    if (r8 != 7) return r8 + 70;

    // Test conditional accumulation
    // cond_accum(5, -1, 3) = 5 + 0 + 3 = 8
    int r9 = cond_accum(5, -1, 3);
    if (r9 != 8) return r9 + 80;

    // cond_accum(-1, -2, -3) = 0
    int r10 = cond_accum(-1, -2, -3);
    if (r10 != 0) return 90;

    return 100;
}
