// INTEGRATION-TEST
// EXPECT: 15
// SKIP: Register allocation fails at -O2 (works at -O1 via tail call optimization)

// Simple recursion - sum from 1 to n
int sum_to_n(int n) {
    if (n <= 0) return 0;
    return n + sum_to_n(n - 1);
}

int test_main(void) {
    return sum_to_n(5);  // 5+4+3+2+1 = 15
}
