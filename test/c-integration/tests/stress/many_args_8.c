// INTEGRATION-TEST
// EXPECT: 360

// Function with 8 arguments (3 in regs, 5 on stack)
int sum8(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a + b + c + d + e + f + g + h;
}

int test_main(void) {
    return sum8(10, 20, 30, 40, 50, 60, 70, 80);  // 360
}
