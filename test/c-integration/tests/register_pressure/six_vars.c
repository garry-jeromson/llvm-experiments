// INTEGRATION-TEST
// EXPECT: 210

// Six simultaneous live variables
int test_main(void) {
    int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60;
    return a + b + c + d + e + f;
}
