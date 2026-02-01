// INTEGRATION-TEST
// EXPECT: 550

// Ten simultaneous live variables
int test_main(void) {
    int a = 10, b = 20, c = 30, d = 40, e = 50;
    int f = 60, g = 70, h = 80, i = 90, j = 100;
    return a + b + c + d + e + f + g + h + i + j;  // 550
}
