// INTEGRATION-TEST
// EXPECT: 360

// Eight phi nodes at control flow merge
int test_main(void) {
    int a, b, c, d, e, f, g, h;
    int cond = 1;

    if (cond) {
        a = 10; b = 20; c = 30; d = 40;
        e = 50; f = 60; g = 70; h = 80;
    } else {
        a = 1; b = 2; c = 3; d = 4;
        e = 5; f = 6; g = 7; h = 8;
    }
    return a + b + c + d + e + f + g + h;  // 360
}
