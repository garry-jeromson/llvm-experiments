// INTEGRATION-TEST
// EXPECT: 25

// Branch inside loop with multiple accumulators
int test_main(void) {
    int even_sum = 0;
    int odd_sum = 0;

    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            even_sum += i;  // 0+2+4+6+8 = 20
        } else {
            odd_sum += i;   // 1+3+5+7+9 = 25
        }
    }
    return odd_sum;  // 25
}
