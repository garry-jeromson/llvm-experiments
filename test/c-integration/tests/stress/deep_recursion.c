// INTEGRATION-TEST
// EXPECT: 5

// Recursive fibonacci (tests stack frames and recursion)
int fib(int n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

int test_main(void) {
    return fib(5);  // 5
}
