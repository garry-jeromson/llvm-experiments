// INTEGRATION-TEST
// EXPECT: 45

int test_main(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum = sum + i;
    }
    return sum;  // 0+1+2+...+9 = 45
}
