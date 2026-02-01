// INTEGRATION-TEST
// EXPECT: 60

// Three phi nodes at control flow merge
int test_main(void) {
    int a, b, c;
    int cond = 1;

    if (cond) {
        a = 10;
        b = 20;
        c = 30;
    } else {
        a = 1;
        b = 2;
        c = 3;
    }
    // Three phi nodes merge here
    return a + b + c;  // 60
}
