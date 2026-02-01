// INTEGRATION-TEST
// EXPECT: 55

// Iterative fibonacci to avoid recursion complexity
int test_main(void) {
    int a = 0;
    int b = 1;
    for (int i = 0; i < 10; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    return a;  // fib(10) = 55
}
