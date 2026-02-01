// INTEGRATION-TEST
// EXPECT: 360

// Eight simultaneous live variables - extreme pressure
int test_main(void) {
    int a = 10, b = 20, c = 30, d = 40;
    int e = 50, f = 60, g = 70, h = 80;
    return a + b + c + d + e + f + g + h;
}
