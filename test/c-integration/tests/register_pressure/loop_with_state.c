// INTEGRATION-TEST
// EXPECT: 143

// Loop maintaining multiple state variables
int test_main(void) {
    int sum = 0;
    int prev = 0;
    int curr = 1;

    // Fibonacci-like accumulation
    for (int i = 0; i < 10; i++) {
        sum += curr;
        int next = prev + curr;
        prev = curr;
        curr = next;
    }
    // curr sequence: 1, 1, 2, 3, 5, 8, 13, 21, 34, 55
    // sum = 1+1+2+3+5+8+13+21+34+55 = 143
    return sum;
}
