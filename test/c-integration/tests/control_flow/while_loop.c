// INTEGRATION-TEST
// EXPECT: 55

int test_main(void) {
    int sum = 0;
    int i = 1;
    while (i <= 10) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;  // 1+2+3+...+10 = 55
}
