// INTEGRATION-TEST
// EXPECT: 210

// Six phi nodes at control flow merge - stress test
int test_main(void) {
    int a, b, c, d, e, f;
    int cond = 1;

    if (cond) {
        a = 10; b = 20; c = 30; d = 40; e = 50; f = 60;
    } else {
        a = 1; b = 2; c = 3; d = 4; e = 5; f = 6;
    }
    // Six phi nodes merge here
    return a + b + c + d + e + f;  // 210
}
