// INTEGRATION-TEST
// EXPECT: 125
// Test default arguments

int compute(int a, int b = 10, int c = 5) {
    return a + b + c;
}

extern "C" int test_main(void) {
    int r1 = compute(20);        // 20 + 10 + 5 = 35
    int r2 = compute(10, 15);    // 10 + 15 + 5 = 30
    int r3 = compute(30, 20, 10);// 30 + 20 + 10 = 60
    return r1 + r2 + r3;         // 35 + 30 + 60 = 125
}
