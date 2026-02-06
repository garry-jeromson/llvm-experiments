// INTEGRATION-TEST
// EXPECT: 25
int test_main(void) {
    int sum = 0;
    for (int i = 1; i < 10; i++) {
        if ((i & 1) == 0) continue;
        sum += i;
    }
    return sum;
}
