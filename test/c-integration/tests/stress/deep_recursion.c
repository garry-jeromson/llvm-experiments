// INTEGRATION-TEST
// EXPECT: 5
// SKIP: At -O2, compiler uses fixed DP locations for locals that get clobbered by recursive calls. At -O1, tail call optimization avoids this but fibonacci is not tail-recursive.

// Recursive fibonacci (tests stack frames and recursion)
int fib(int n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

int test_main(void) {
    return fib(5);  // 5
}
