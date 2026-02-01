// INTEGRATION-TEST
// EXPECT: 35

int double_it(int x) {
    return x + x;
}

int test_main(void) {
    int a = 5;
    int b = double_it(a);      // 10
    int c = double_it(b);      // 20
    return a + b + c;          // 5 + 10 + 20 = 35
}
