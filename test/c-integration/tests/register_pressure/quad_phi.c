// INTEGRATION-TEST
// EXPECT: 100

// Four phi nodes at control flow merge
int test_main(void) {
    int a, b, c, d;
    int cond = 1;

    if (cond) {
        a = 10;
        b = 20;
        c = 30;
        d = 40;
    } else {
        a = 1;
        b = 2;
        c = 3;
        d = 4;
    }
    // Four phi nodes merge here
    return a + b + c + d;  // 100
}
